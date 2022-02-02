// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>
    SPDX-FileCopyrightText: 2008 Norbert Frese <nf2@scheinwelt.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkmanager.h"
#include "kbookmarkdialog.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarks_debug.h"
#ifndef KBOOKMARKS_NO_DBUS
#include "kbookmarkmanageradaptor_p.h"
#endif

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTextCodec>
#include <QTextStream>
#ifndef KBOOKMARKS_NO_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#endif
#include <QApplication>
#include <QMessageBox>
#include <QReadWriteLock>
#include <QThread>

#include <KBackup>
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
namespace Strings
{
QString bookmarkChangeNotifyInterface()
{
    return QStringLiteral("org.kde.KIO.KBookmarkManager");
}
QString piData()
{
    return QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"");
}
}
}

class KBookmarkManagerList : public QList<KBookmarkManager *>
{
public:
    KBookmarkManagerList();
    ~KBookmarkManagerList()
    {
        cleanup();
    }
    void cleanup()
    {
        QList<KBookmarkManager *> copy = *this;
        qDeleteAll(copy); // auto-delete functionality
        clear();
    }

    QReadWriteLock lock;
};

Q_GLOBAL_STATIC(KBookmarkManagerList, s_pSelf)

static void deleteManagers()
{
    if (s_pSelf.exists()) {
        s_pSelf->cleanup();
    }
}

KBookmarkManagerList::KBookmarkManagerList()
{
    // Delete the KBookmarkManagers while qApp exists, since we interact with the DBus thread
    qAddPostRoutine(deleteManagers);
}

class KBookmarkMap : private KBookmarkGroupTraverser
{
public:
    KBookmarkMap()
        : m_mapNeedsUpdate(true)
    {
    }
    void setNeedsUpdate()
    {
        m_mapNeedsUpdate = true;
    }
    void update(KBookmarkManager *);
    QList<KBookmark> find(const QString &url) const
    {
        return m_bk_map.value(url);
    }

private:
    void visit(const KBookmark &) override;
    void visitEnter(const KBookmarkGroup &) override
    {
        ;
    }
    void visitLeave(const KBookmarkGroup &) override
    {
        ;
    }

private:
    typedef QList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    bool m_mapNeedsUpdate;
};

void KBookmarkMap::update(KBookmarkManager *manager)
{
    if (m_mapNeedsUpdate) {
        m_mapNeedsUpdate = false;

        m_bk_map.clear();
        KBookmarkGroup root = manager->root();
        traverse(root);
    }
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute(QStringLiteral("href"))].append(bk);
    }
}

// #########################
// KBookmarkManagerPrivate
class KBookmarkManagerPrivate
{
public:
    KBookmarkManagerPrivate(bool bDocIsloaded, const QString &dbusObjectName = QString())
        : m_doc(QStringLiteral("xbel"))
        , m_dbusObjectName(dbusObjectName)
        , m_docIsLoaded(bDocIsloaded)
        , m_update(false)
        , m_dialogAllowed(true)
        , m_dialogParent(nullptr)
        , m_browserEditor(false)
        , m_typeExternal(false)
        , m_dirWatch(nullptr)
    {
    }

    ~KBookmarkManagerPrivate()
    {
        delete m_dirWatch;
    }

    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    QString m_bookmarksFile;
    QString m_dbusObjectName;
    mutable bool m_docIsLoaded;
    bool m_update;
    bool m_dialogAllowed;
    QWidget *m_dialogParent;

    bool m_browserEditor;
    QString m_editorCaption;

    bool m_typeExternal;
    KDirWatch *m_dirWatch; // for external bookmark files

    KBookmarkMap m_map;
};

// ################
// KBookmarkManager

static KBookmarkManager *lookupExisting(const QString &bookmarksFile)
{
    for (KBookmarkManagerList::ConstIterator bmit = s_pSelf()->constBegin(), bmend = s_pSelf()->constEnd(); bmit != bmend; ++bmit) {
        if ((*bmit)->path() == bookmarksFile) {
            return *bmit;
        }
    }
    return nullptr;
}

KBookmarkManager *KBookmarkManager::managerForFile(const QString &bookmarksFile, const QString &dbusObjectName)
{
    KBookmarkManager *mgr(nullptr);
    {
        QReadLocker readLock(&s_pSelf()->lock);
        mgr = lookupExisting(bookmarksFile);
        if (mgr) {
            return mgr;
        }
    }

    QWriteLocker writeLock(&s_pSelf()->lock);
    mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager(bookmarksFile, dbusObjectName);
    s_pSelf()->append(mgr);
    return mgr;
}

KBookmarkManager *KBookmarkManager::managerForExternalFile(const QString &bookmarksFile)
{
    KBookmarkManager *mgr(nullptr);
    {
        QReadLocker readLock(&s_pSelf()->lock);
        mgr = lookupExisting(bookmarksFile);
        if (mgr) {
            return mgr;
        }
    }

    QWriteLocker writeLock(&s_pSelf()->lock);
    mgr = lookupExisting(bookmarksFile);
    if (mgr) {
        return mgr;
    }

    mgr = new KBookmarkManager(bookmarksFile);
    s_pSelf()->append(mgr);
    return mgr;
}

// principally used for filtered toolbars
KBookmarkManager *KBookmarkManager::createTempManager()
{
    KBookmarkManager *mgr = new KBookmarkManager();
    s_pSelf()->append(mgr);
    return mgr;
}

static QDomElement createXbelTopLevelElement(QDomDocument &doc)
{
    QDomElement topLevel = doc.createElement(QStringLiteral("xbel"));
    topLevel.setAttribute(QStringLiteral("xmlns:mime"), QStringLiteral("http://www.freedesktop.org/standards/shared-mime-info"));
    topLevel.setAttribute(QStringLiteral("xmlns:bookmark"), QStringLiteral("http://www.freedesktop.org/standards/desktop-bookmarks"));
    topLevel.setAttribute(QStringLiteral("xmlns:kdepriv"), QStringLiteral("http://www.kde.org/kdepriv"));
    doc.appendChild(topLevel);
    doc.insertBefore(doc.createProcessingInstruction(QStringLiteral("xml"), Strings::piData()), topLevel);
    return topLevel;
}

KBookmarkManager::KBookmarkManager(const QString &bookmarksFile, const QString &dbusObjectName)
    : d(new KBookmarkManagerPrivate(false, dbusObjectName))
{
    if (dbusObjectName.isNull()) { // get dbusObjectName from file
        if (QFile::exists(d->m_bookmarksFile)) {
            parse(); // sets d->m_dbusObjectName
        }
    }

    init(QLatin1String("/KBookmarkManager/") + d->m_dbusObjectName);

    d->m_update = true;

    Q_ASSERT(!bookmarksFile.isEmpty());
    d->m_bookmarksFile = bookmarksFile;

    if (!QFile::exists(d->m_bookmarksFile)) {
        QDomElement topLevel = createXbelTopLevelElement(d->m_doc);
        topLevel.setAttribute(QStringLiteral("dbusName"), dbusObjectName);
        d->m_docIsLoaded = true;
    }
}

KBookmarkManager::KBookmarkManager(const QString &bookmarksFile)
    : d(new KBookmarkManagerPrivate(false))
{
    // use QFileSystemWatcher to monitor this bookmarks file
    d->m_typeExternal = true;
    d->m_update = true;

    Q_ASSERT(!bookmarksFile.isEmpty());
    d->m_bookmarksFile = bookmarksFile;

    if (!QFile::exists(d->m_bookmarksFile)) {
        createXbelTopLevelElement(d->m_doc);
    } else {
        parse();
    }
    d->m_docIsLoaded = true;

    // start KDirWatch
    d->m_dirWatch = new KDirWatch;
    d->m_dirWatch->addFile(d->m_bookmarksFile);
    QObject::connect(d->m_dirWatch, &KDirWatch::dirty, this, &KBookmarkManager::slotFileChanged);
    QObject::connect(d->m_dirWatch, &KDirWatch::created, this, &KBookmarkManager::slotFileChanged);
    QObject::connect(d->m_dirWatch, &KDirWatch::deleted, this, &KBookmarkManager::slotFileChanged);

    // qCDebug(KBOOKMARKS_LOG) << "starting KDirWatch for" << d->m_bookmarksFile;
}

KBookmarkManager::KBookmarkManager()
    : d(new KBookmarkManagerPrivate(true))
{
    init(QStringLiteral("/KBookmarkManager/generated"));
    d->m_update = false; // TODO - make it read/write

    createXbelTopLevelElement(d->m_doc);
}

void KBookmarkManager::init(const QString &dbusPath)
{
#ifndef KBOOKMARKS_NO_DBUS
    // A KBookmarkManager without a dbus name is a temporary one, like those used by importers;
    // no need to register them to dbus
    if (dbusPath != QLatin1String("/KBookmarkManager/") && dbusPath != QLatin1String("/KBookmarkManager/generated")) {
        new KBookmarkManagerAdaptor(this);
        QDBusConnection::sessionBus().registerObject(dbusPath, this);

        QDBusConnection::sessionBus().connect(QString(),
                                              dbusPath,
                                              Strings::bookmarkChangeNotifyInterface(),
                                              QStringLiteral("bookmarksChanged"),
                                              this,
                                              SLOT(notifyChanged(QString, QDBusMessage)));
        QDBusConnection::sessionBus()
            .connect(QString(), dbusPath, Strings::bookmarkChangeNotifyInterface(), QStringLiteral("bookmarkConfigChanged"), this, SLOT(notifyConfigChanged()));
    }
#endif
}

void KBookmarkManager::startKEditBookmarks(const QStringList &args)
{
    bool success = false;
    const QString exec = QStandardPaths::findExecutable(QStringLiteral(KEDITBOOKMARKS_BINARY));
    if (!exec.isEmpty()) {
        success = QProcess::startDetached(exec, args);
    }

    if (!success) {
        QString err =
            tr("Cannot launch keditbookmarks.\n\n"
               "Most likely you do not have keditbookmarks currently installed");

        if (d->m_dialogAllowed && qobject_cast<QApplication *>(qApp) && QThread::currentThread() == qApp->thread()) {
            QMessageBox::warning(QApplication::activeWindow(), QApplication::applicationName(), err);
        }

        qCWarning(KBOOKMARKS_LOG) << QStringLiteral("Failed to start keditbookmarks");
        Q_EMIT this->error(err);
    }
}

void KBookmarkManager::slotFileChanged(const QString &path)
{
    if (path == d->m_bookmarksFile) {
        // qCDebug(KBOOKMARKS_LOG) << "file changed (KDirWatch) " << path ;
        // Reparse
        parse();
        // Tell our GUI
        // (emit where group is "" to directly mark the root menu as dirty)
        Q_EMIT changed(QLatin1String(""), QString());
    }
}

KBookmarkManager::~KBookmarkManager()
{
    if (!s_pSelf.isDestroyed()) {
        s_pSelf()->removeAll(this);
    }
}

bool KBookmarkManager::autoErrorHandlingEnabled() const
{
    return d->m_dialogAllowed;
}

void KBookmarkManager::setAutoErrorHandlingEnabled(bool enable, QWidget *parent)
{
    d->m_dialogAllowed = enable;
    d->m_dialogParent = parent;
}

void KBookmarkManager::setUpdate(bool update)
{
    d->m_update = update;
}

QDomDocument KBookmarkManager::internalDocument() const
{
    if (!d->m_docIsLoaded) {
        parse();
        d->m_toolbarDoc.clear();
    }
    return d->m_doc;
}

void KBookmarkManager::parse() const
{
    d->m_docIsLoaded = true;
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::parse " << d->m_bookmarksFile;
    QFile file(d->m_bookmarksFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(KBOOKMARKS_LOG) << "Can't open" << d->m_bookmarksFile;
        d->m_doc = QDomDocument(QStringLiteral("xbel"));
        createXbelTopLevelElement(d->m_doc);
        return;
    }
    d->m_doc = QDomDocument(QStringLiteral("xbel"));
    d->m_doc.setContent(&file);

    if (d->m_doc.documentElement().isNull()) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::parse : main tag is missing, creating default " << d->m_bookmarksFile;
        QDomElement element = d->m_doc.createElement(QStringLiteral("xbel"));
        d->m_doc.appendChild(element);
    }

    QDomElement docElem = d->m_doc.documentElement();

    QString mainTag = docElem.tagName();
    if (mainTag != QLatin1String("xbel")) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::parse : unknown main tag " << mainTag;
    }

    if (d->m_dbusObjectName.isNull()) {
        d->m_dbusObjectName = docElem.attribute(QStringLiteral("dbusName"));
    } else if (docElem.attribute(QStringLiteral("dbusName")) != d->m_dbusObjectName) {
        docElem.setAttribute(QStringLiteral("dbusName"), d->m_dbusObjectName);
        save();
    }

    QDomNode n = d->m_doc.documentElement().previousSibling();
    if (n.isProcessingInstruction()) {
        QDomProcessingInstruction pi = n.toProcessingInstruction();
        pi.parentNode().removeChild(pi);
    }

    QDomProcessingInstruction pi;
    pi = d->m_doc.createProcessingInstruction(QStringLiteral("xml"), Strings::piData());
    d->m_doc.insertBefore(pi, docElem);

    file.close();

    d->m_map.setNeedsUpdate();
}

bool KBookmarkManager::save(bool toolbarCache) const
{
    return saveAs(d->m_bookmarksFile, toolbarCache);
}

bool KBookmarkManager::saveAs(const QString &filename, bool toolbarCache) const
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::save " << filename;

    // Save the bookmark toolbar folder for quick loading
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QLatin1String(".tbcache");
    if (toolbarCache && !root().isToolbarGroup()) {
        QSaveFile cacheFile(cacheFilename);
        if (cacheFile.open(QIODevice::WriteOnly)) {
            QString str;
            QTextStream stream(&str, QIODevice::WriteOnly);
            stream << root().findToolbar();
            const QByteArray cstr = str.toUtf8();
            cacheFile.write(cstr.data(), cstr.length());
            cacheFile.commit();
        }
    } else { // remove any (now) stale cache
        QFile::remove(cacheFilename);
    }

    // Create parent dirs
    QFileInfo info(filename);
    QDir().mkpath(info.absolutePath());

    QSaveFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        KBackup::simpleBackupFile(file.fileName(), QString(), QStringLiteral(".bak"));
        QTextStream stream(&file);
        // In Qt6 it's UTF-8 by default
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
        stream << internalDocument().toString();
        stream.flush();
        if (file.commit()) {
            return true;
        }
    }

    static int hadSaveError = false;
    if (!hadSaveError) {
        QString err = tr("Unable to save bookmarks in %1. Reported error was: %2. "
                         "This error message will only be shown once. The cause "
                         "of the error needs to be fixed as quickly as possible, "
                         "which is most likely a full hard drive.")
                          .arg(filename, file.errorString());

        if (d->m_dialogAllowed && qobject_cast<QApplication *>(qApp) && QThread::currentThread() == qApp->thread()) {
            QMessageBox::critical(QApplication::activeWindow(), QApplication::applicationName(), err);
        }

        qCCritical(KBOOKMARKS_LOG)
            << QStringLiteral("Unable to save bookmarks in %1. File reported the following error-code: %2.").arg(filename).arg(file.error());
        Q_EMIT const_cast<KBookmarkManager *>(this)->error(err);
    }
    hadSaveError = true;
    return false;
}

QString KBookmarkManager::path() const
{
    return d->m_bookmarksFile;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmarkGroup KBookmarkManager::toolbar()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar begin";
    // Only try to read from a toolbar cache if the full document isn't loaded
    if (!d->m_docIsLoaded) {
        // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar trying cache";
        const QString cacheFilename = d->m_bookmarksFile + QLatin1String(".tbcache");
        QFileInfo bmInfo(d->m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (d->m_toolbarDoc.isNull() && QFile::exists(cacheFilename) && bmInfo.lastModified() < cacheInfo.lastModified()) {
            // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar reading file";
            QFile file(cacheFilename);

            if (file.open(QIODevice::ReadOnly)) {
                d->m_toolbarDoc = QDomDocument(QStringLiteral("cache"));
                d->m_toolbarDoc.setContent(&file);
                // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar opened";
            }
        }
        if (!d->m_toolbarDoc.isNull()) {
            // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar returning element";
            QDomElement elem = d->m_toolbarDoc.firstChild().toElement();
            return KBookmarkGroup(elem);
        }
    }

    // Fallback to the normal way if there is no cache or if the bookmark file
    // is already loaded
    QDomElement elem = root().findToolbar();
    if (elem.isNull()) {
        // Root is the bookmark toolbar if none has been set.
        // Make it explicit to speed up invocations of findToolbar()
        root().internalElement().setAttribute(QStringLiteral("toolbar"), QStringLiteral("yes"));
        return root();
    } else {
        return KBookmarkGroup(elem);
    }
}

KBookmark KBookmarkManager::findByAddress(const QString &address)
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::findByAddress " << address;
    KBookmark result = root();
    // The address is something like /5/10/2+
    static const QRegularExpression separator(QStringLiteral("[/+]"));
    const QStringList addresses = address.split(separator, Qt::SkipEmptyParts);
    // qCWarning(KBOOKMARKS_LOG) << addresses.join(",");
    for (QStringList::const_iterator it = addresses.begin(); it != addresses.end();) {
        bool append = ((*it) == QLatin1String("+"));
        uint number = (*it).toUInt();
        Q_ASSERT(result.isGroup());
        KBookmarkGroup group = result.toGroup();
        KBookmark bk = group.first();
        KBookmark lbk = bk; // last non-null bookmark
        for (uint i = 0; ((i < number) || append) && !bk.isNull(); ++i) {
            lbk = bk;
            bk = group.next(bk);
            // qCWarning(KBOOKMARKS_LOG) << i;
        }
        it++;
        // qCWarning(KBOOKMARKS_LOG) << "found section";
        result = bk;
    }
    if (result.isNull()) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::findByAddress: couldn't find item " << address;
    }
    // qCWarning(KBOOKMARKS_LOG) << "found " << result.address();
    return result;
}

void KBookmarkManager::emitChanged()
{
    emitChanged(root());
}

void KBookmarkManager::emitChanged(const KBookmarkGroup &group)
{
    (void)save(); // KDE5 TODO: emitChanged should return a bool? Maybe rename it to saveAndEmitChanged?

    // Tell the other processes too
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::emitChanged : broadcasting change " << group.address();

    Q_EMIT bookmarksChanged(group.address());

    // We do get our own broadcast, so no need for this anymore
    // emit changed( group );
}

void KBookmarkManager::emitConfigChanged()
{
    Q_EMIT bookmarkConfigChanged();
}

void KBookmarkManager::notifyCompleteChange(const QString &caller) // DBUS call
{
    if (!d->m_update) {
        return;
    }

    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::notifyCompleteChange";
    // The bk editor tells us we should reload everything
    // Reparse
    parse();
    // Tell our GUI
    // (emit where group is "" to directly mark the root menu as dirty)
    Q_EMIT changed(QLatin1String(""), caller);
}

void KBookmarkManager::notifyConfigChanged() // DBUS call
{
    // qCDebug(KBOOKMARKS_LOG) << "reloaded bookmark config!";
    KBookmarkSettings::self()->readSettings();
    parse(); // reload, and thusly recreate the menus
    Q_EMIT configChanged();
}

#ifndef KBOOKMARKS_NO_DBUS
void KBookmarkManager::notifyChanged(const QString &groupAddress, const QDBusMessage &msg) // DBUS call
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::notifyChanged ( "<<groupAddress<<")";
    if (!d->m_update) {
        return;
    }

    // Reparse (the whole file, no other choice)
    // if someone else notified us
    if (msg.service() != QDBusConnection::sessionBus().baseService()) {
        parse();
    }

    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::notifyChanged " << groupAddress;
    // KBookmarkGroup group = findByAddress( groupAddress ).toGroup();
    // Q_ASSERT(!group.isNull());
    Q_EMIT changed(groupAddress, QString());
}
#endif

void KBookmarkManager::setEditorOptions(const QString &caption, bool browser)
{
    d->m_editorCaption = caption;
    d->m_browserEditor = browser;
}

void KBookmarkManager::slotEditBookmarks()
{
    QStringList args;
    if (!d->m_editorCaption.isEmpty()) {
        args << QStringLiteral("--customcaption") << d->m_editorCaption;
    }
    if (!d->m_browserEditor) {
        args << QStringLiteral("--nobrowser");
    }
    if (!d->m_dbusObjectName.isEmpty()) {
        args << QStringLiteral("--dbusObjectName") << d->m_dbusObjectName;
    }
    args << d->m_bookmarksFile;
    startKEditBookmarks(args);
}

void KBookmarkManager::slotEditBookmarksAtAddress(const QString &address)
{
    QStringList args;
    if (!d->m_editorCaption.isEmpty()) {
        args << QStringLiteral("--customcaption") << d->m_editorCaption;
    }
    if (!d->m_browserEditor) {
        args << QStringLiteral("--nobrowser");
    }
    if (!d->m_dbusObjectName.isEmpty()) {
        args << QStringLiteral("--dbusObjectName") << d->m_dbusObjectName;
    }
    args << QStringLiteral("--address") << address << d->m_bookmarksFile;
    startKEditBookmarks(args);
}

///////
bool KBookmarkManager::updateAccessMetadata(const QString &url)
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    if (list.isEmpty()) {
        return false;
    }

    for (QList<KBookmark>::iterator it = list.begin(); it != list.end(); ++it) {
        (*it).updateAccessMetadata();
    }

    return true;
}

void KBookmarkManager::updateFavicon(const QString &url, const QString & /*faviconurl*/)
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    for (QList<KBookmark>::iterator it = list.begin(); it != list.end(); ++it) {
        // TODO - update favicon data based on faviconurl
        //        but only when the previously used icon
        //        isn't a manually set one.
    }
}

KBookmarkManager *KBookmarkManager::userBookmarksManager()
{
    const QString bookmarksFile =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/konqueror/bookmarks.xml");
    KBookmarkManager *bookmarkManager = KBookmarkManager::managerForFile(bookmarksFile, QStringLiteral("konqueror"));
    QString caption = QGuiApplication::applicationDisplayName();
    if (caption.isEmpty()) {
        caption = QCoreApplication::applicationName();
    }
    bookmarkManager->setEditorOptions(caption, true);
    return bookmarkManager;
}

KBookmarkSettings *KBookmarkSettings::s_self = nullptr;

void KBookmarkSettings::readSettings()
{
    KConfig config(QStringLiteral("kbookmarkrc"), KConfig::NoGlobals);
    KConfigGroup cg(&config, "Bookmarks");

    // add bookmark dialog usage - no reparse
    s_self->m_advancedaddbookmark = cg.readEntry("AdvancedAddBookmarkDialog", false);

    // this one alters the menu, therefore it needs a reparse
    s_self->m_contextmenu = cg.readEntry("ContextMenuActions", true);
}

KBookmarkSettings *KBookmarkSettings::self()
{
    if (!s_self) {
        s_self = new KBookmarkSettings;
        readSettings();
    }
    return s_self;
}

#include "moc_kbookmarkmanager.cpp"
