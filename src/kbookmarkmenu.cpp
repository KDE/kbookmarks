/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"

#include "kbookmarkaction.h"
#include "kbookmarkactionmenu.h"
#include "kbookmarkcontextmenu.h"
#include "kbookmarkdialog.h"
#include "kbookmarkowner.h"
#include "kbookmarks_debug.h"

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
#include <KActionCollection>
#endif
#include <KAuthorized>
#include <KStandardAction>

#include <QApplication>
#include <QMenu>
#include <QStandardPaths>

/********************************************************************/
/********************************************************************/
/********************************************************************/
class KBookmarkMenuPrivate
{
public:
    QAction *newBookmarkFolderAction = nullptr;
    QAction *addBookmarkAction = nullptr;
    QAction *bookmarksToFolderAction = nullptr;
    QAction *editBookmarksAction = nullptr;
    int numberOfOpenTabs = 2;
};

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
KBookmarkMenu::KBookmarkMenu(KBookmarkManager *mgr, KBookmarkOwner *_owner, QMenu *_parentMenu, KActionCollection *actionCollection)
    : QObject()
    , m_actionCollection(actionCollection)
    , d(new KBookmarkMenuPrivate())
    , m_bIsRoot(true)
    , m_pManager(mgr)
    , m_pOwner(_owner)
    , m_parentMenu(_parentMenu)
    , m_parentAddress(QString()) // TODO KBookmarkAdress::root
{
    init();
}

#endif

KBookmarkMenu::KBookmarkMenu(KBookmarkManager *manager, KBookmarkOwner *_owner, QMenu *_parentMenu)
    : QObject()
#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
    , m_actionCollection(new KActionCollection(this))
#endif
    , d(new KBookmarkMenuPrivate())
    , m_bIsRoot(true)
    , m_pManager(manager)
    , m_pOwner(_owner)
    , m_parentMenu(_parentMenu)
    , m_parentAddress(QString()) // TODO KBookmarkAdress::root
{
    // TODO KDE5 find a QMenu equvalnet for this one
    // m_parentMenu->setKeyboardShortcutsEnabled( true );

    init();
}

void KBookmarkMenu::init()
{
    connect(m_parentMenu, &QMenu::aboutToShow, this, &KBookmarkMenu::slotAboutToShow);

    if (KBookmarkSettings::self()->m_contextmenu) {
        m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_parentMenu, &QWidget::customContextMenuRequested, this, &KBookmarkMenu::slotCustomContextMenu);
    }

    connect(m_pManager, &KBookmarkManager::changed, this, &KBookmarkMenu::slotBookmarksChanged);

    m_bDirty = true;
    addActions();
}

void KBookmarkMenu::addActions()
{
    if (m_bIsRoot) {
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
        addEditBookmarks();
    } else {
        if (!m_parentMenu->actions().isEmpty()) {
            m_parentMenu->addSeparator();
        }

        addOpenInTabs();
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
    }
}

KBookmarkMenu::KBookmarkMenu(KBookmarkManager *mgr, KBookmarkOwner *_owner, QMenu *_parentMenu, const QString &parentAddress)
    : QObject()
#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
    , m_actionCollection(new KActionCollection(this))
#endif
    , d(new KBookmarkMenuPrivate())
    , m_bIsRoot(false)
    , m_pManager(mgr)
    , m_pOwner(_owner)
    , m_parentMenu(_parentMenu)
    , m_parentAddress(parentAddress)
{
    connect(_parentMenu, &QMenu::aboutToShow, this, &KBookmarkMenu::slotAboutToShow);
    if (KBookmarkSettings::self()->m_contextmenu) {
        m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_parentMenu, &QWidget::customContextMenuRequested, this, &KBookmarkMenu::slotCustomContextMenu);
    }
    m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
    qDeleteAll(m_lstSubMenus);
    qDeleteAll(m_actions);
}

void KBookmarkMenu::ensureUpToDate()
{
    slotAboutToShow();
}

void KBookmarkMenu::setNumberOfOpenTabs(int numberOfOpenTabs)
{
    if (numberOfOpenTabs == d->numberOfOpenTabs) {
        return;
    }
    m_bDirty = (d->numberOfOpenTabs < 2) != (numberOfOpenTabs < 2);
    d->numberOfOpenTabs = numberOfOpenTabs;
}

int KBookmarkMenu::numberOfOpenTabs() const
{
    return d->numberOfOpenTabs;
}

void KBookmarkMenu::slotAboutToShow()
{
    // Did the bookmarks change since the last time we showed them ?
    if (m_bDirty) {
        m_bDirty = false;
        clear();
        refill();
        m_parentMenu->adjustSize();
    }
}

void KBookmarkMenu::slotCustomContextMenu(const QPoint &pos)
{
    QAction *action = m_parentMenu->actionAt(pos);
    QMenu *menu = contextMenu(action);
    if (!menu) {
        return;
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(m_parentMenu->mapToGlobal(pos));
}

QMenu *KBookmarkMenu::contextMenu(QAction *action)
{
    KBookmarkActionInterface *act = dynamic_cast<KBookmarkActionInterface *>(action);
    if (!act) {
        return nullptr;
    }
    return new KBookmarkContextMenu(act->bookmark(), m_pManager, m_pOwner);
}

bool KBookmarkMenu::isRoot() const
{
    return m_bIsRoot;
}

bool KBookmarkMenu::isDirty() const
{
    return m_bDirty;
}

QString KBookmarkMenu::parentAddress() const
{
    return m_parentAddress;
}

KBookmarkManager *KBookmarkMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner *KBookmarkMenu::owner() const
{
    return m_pOwner;
}

QMenu *KBookmarkMenu::parentMenu() const
{
    return m_parentMenu;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::slotBookmarksChanged(const QString &groupAddress)
{
    qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotBookmarksChanged groupAddress: " << groupAddress;
    if (groupAddress == m_parentAddress) {
        // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress;
        m_bDirty = true;
    } else {
        // Iterate recursively into child menus
        for (QList<KBookmarkMenu *>::iterator it = m_lstSubMenus.begin(), end = m_lstSubMenus.end(); it != end; ++it) {
            (*it)->slotBookmarksChanged(groupAddress);
        }
    }
}

void KBookmarkMenu::clear()
{
    qDeleteAll(m_lstSubMenus);
    m_lstSubMenus.clear();

    for (QList<QAction *>::iterator it = m_actions.begin(), end = m_actions.end(); it != end; ++it) {
        m_parentMenu->removeAction(*it);
        delete *it;
    }

    m_parentMenu->clear();
    m_actions.clear();
}

void KBookmarkMenu::refill()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::refill()";
    if (m_bIsRoot) {
        addActions();
    }
    fillBookmarks();
    if (!m_bIsRoot) {
        addActions();
    }
}

void KBookmarkMenu::addOpenInTabs()
{
    if (!m_pOwner || !m_pOwner->supportsTabs() || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    const QString title = tr("Open Folder in Tabs", "@action:inmenu");

    QAction *paOpenFolderInTabs = new QAction(title, this);
    paOpenFolderInTabs->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    paOpenFolderInTabs->setToolTip(tr("Open all bookmarks in this folder as a new tab", "@info:tooltip"));
    paOpenFolderInTabs->setStatusTip(paOpenFolderInTabs->toolTip());
    connect(paOpenFolderInTabs, &QAction::triggered, this, &KBookmarkMenu::slotOpenFolderInTabs);

    m_parentMenu->addAction(paOpenFolderInTabs);
    m_actions.append(paOpenFolderInTabs);
}

void KBookmarkMenu::addAddBookmarksList()
{
    if (!m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !m_pOwner->supportsTabs() || (d->numberOfOpenTabs < 2)
        || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->bookmarksToFolderAction) {
        const QString title = tr("Bookmark Tabs as Folder...", "@action:inmenu");
        d->bookmarksToFolderAction = new QAction(title, this);

        if (m_bIsRoot) {
            d->bookmarksToFolderAction->setObjectName(QStringLiteral("add_bookmarks_list"));
        }

        d->bookmarksToFolderAction->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new-list")));
        d->bookmarksToFolderAction->setToolTip(tr("Add a folder of bookmarks for all open tabs", "@info:tooltip"));
        d->bookmarksToFolderAction->setStatusTip(d->bookmarksToFolderAction->toolTip());
        connect(d->bookmarksToFolderAction, &QAction::triggered, this, &KBookmarkMenu::slotAddBookmarksList);

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
        if (m_actionCollection) {
            m_actionCollection->addAction(d->bookmarksToFolderAction->objectName(), d->bookmarksToFolderAction);
        }
#endif
    }

    m_parentMenu->addAction(d->bookmarksToFolderAction);
}

void KBookmarkMenu::addAddBookmark()
{
    if (!m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->addBookmarkAction) {
        d->addBookmarkAction = KStandardAction::addBookmark(this, SLOT(slotAddBookmark()), this);
        if (m_bIsRoot) {
            d->addBookmarkAction->setObjectName(QStringLiteral("add_bookmark"));
        }

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
        if (m_actionCollection) {
            m_actionCollection->addAction(d->addBookmarkAction->objectName(), d->addBookmarkAction);
        }
#endif

        if (!m_bIsRoot) {
            d->addBookmarkAction->setShortcut(QKeySequence());
        }
    }

    m_parentMenu->addAction(d->addBookmarkAction);
}

void KBookmarkMenu::addEditBookmarks()
{
    if ((m_pOwner && !m_pOwner->enableOption(KBookmarkOwner::ShowEditBookmark))
        || QStandardPaths::findExecutable(QStringLiteral(KEDITBOOKMARKS_BINARY)).isEmpty() || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    d->editBookmarksAction = KStandardAction::editBookmarks(m_pManager, SLOT(slotEditBookmarks()), this);
    d->editBookmarksAction->setObjectName(QStringLiteral("edit_bookmarks"));

    m_parentMenu->addAction(d->editBookmarksAction);
    d->editBookmarksAction->setToolTip(tr("Edit your bookmark collection in a separate window", "@info:tooltip"));
    d->editBookmarksAction->setStatusTip(d->editBookmarksAction->toolTip());

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
    if (m_actionCollection) {
        m_actionCollection->addAction(d->editBookmarksAction->objectName(), d->editBookmarksAction);
    }
#endif
}

void KBookmarkMenu::addNewFolder()
{
    if (!m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->newBookmarkFolderAction) {
        d->newBookmarkFolderAction = new QAction(tr("New Bookmark Folder...", "@action:inmenu"), this);
        d->newBookmarkFolderAction->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
        d->newBookmarkFolderAction->setToolTip(tr("Create a new bookmark folder in this menu", "@info:tooltip"));
        d->newBookmarkFolderAction->setStatusTip(d->newBookmarkFolderAction->toolTip());

        if (m_bIsRoot) {
            d->newBookmarkFolderAction->setObjectName(QStringLiteral("new_bookmark_folder"));
        }

        connect(d->newBookmarkFolderAction, &QAction::triggered, this, &KBookmarkMenu::slotNewFolder);
    }

    m_parentMenu->addAction(d->newBookmarkFolderAction);
}

void KBookmarkMenu::fillBookmarks()
{
    KBookmarkGroup parentBookmark = m_pManager->findByAddress(m_parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());

    if (m_bIsRoot && !parentBookmark.first().isNull()) { // at least one bookmark
        m_parentMenu->addSeparator();
    }

    for (KBookmark bm = parentBookmark.first(); !bm.isNull(); bm = parentBookmark.next(bm)) {
        m_parentMenu->addAction(actionForBookmark(bm));
    }
}

QAction *KBookmarkMenu::actionForBookmark(const KBookmark &bm)
{
    if (bm.isGroup()) {
        // qCDebug(KBOOKMARKS_LOG) << "Creating bookmark submenu named " << bm.text();
        KActionMenu *actionMenu = new KBookmarkActionMenu(bm, this);
        m_actions.append(actionMenu);
        KBookmarkMenu *subMenu = new KBookmarkMenu(m_pManager, m_pOwner, actionMenu->menu(), bm.address());
        m_lstSubMenus.append(subMenu);
        return actionMenu;
    } else if (bm.isSeparator()) {
        QAction *sa = new QAction(this);
        sa->setSeparator(true);
        m_actions.append(sa);
        return sa;
    } else {
        // qCDebug(KBOOKMARKS_LOG) << "Creating bookmark menu item for " << bm.text();
        QAction *action = new KBookmarkAction(bm, m_pOwner, this);
        m_actions.append(action);
        return action;
    }
}

void KBookmarkMenu::slotAddBookmarksList()
{
    if (!m_pOwner || !m_pOwner->supportsTabs()) {
        return;
    }

    KBookmarkGroup parentBookmark = m_pManager->findByAddress(m_parentAddress).toGroup();

    KBookmarkDialog *dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->addBookmarks(m_pOwner->currentBookmarkList(), QLatin1String(""), parentBookmark);
    delete dlg;
}

void KBookmarkMenu::slotAddBookmark()
{
    if (!m_pOwner) {
        return;
    }
    if (m_pOwner->currentTitle().isEmpty() && m_pOwner->currentUrl().isEmpty()) {
        return;
    }
    KBookmarkGroup parentBookmark = m_pManager->findByAddress(m_parentAddress).toGroup();

    if (KBookmarkSettings::self()->m_advancedaddbookmark) {
        KBookmarkDialog *dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
        dlg->addBookmark(m_pOwner->currentTitle(), m_pOwner->currentUrl(), m_pOwner->currentIcon(), parentBookmark);
        delete dlg;
    } else {
        parentBookmark.addBookmark(m_pOwner->currentTitle(), m_pOwner->currentUrl(), m_pOwner->currentIcon());
        m_pManager->emitChanged(parentBookmark);
    }
}

void KBookmarkMenu::slotOpenFolderInTabs()
{
    m_pOwner->openFolderinTabs(m_pManager->findByAddress(m_parentAddress).toGroup());
}

void KBookmarkMenu::slotNewFolder()
{
    if (!m_pOwner) {
        return; // this view doesn't handle bookmarks...
    }
    KBookmarkGroup parentBookmark = m_pManager->findByAddress(m_parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmarkDialog *dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->createNewFolder(QLatin1String(""), parentBookmark);
    delete dlg;
}

QAction *KBookmarkMenu::addBookmarkAction() const
{
    return d->addBookmarkAction;
}

QAction *KBookmarkMenu::bookmarkTabsAsFolderAction() const
{
    return d->bookmarksToFolderAction;
}

QAction *KBookmarkMenu::newBookmarkFolderAction() const
{
    return d->newBookmarkFolderAction;
}

QAction *KBookmarkMenu::editBookmarksAction() const
{
    return d->editBookmarksAction;
}
