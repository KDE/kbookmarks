/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 Daniel Teske <teske@squorn.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkcontextmenu.h"
#include "kbookmarkowner.h"
#include "kbookmarkmanager.h"
#include "kbookmarkdialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QMimeData>
#include <QClipboard>

KBookmarkContextMenu::KBookmarkContextMenu(const KBookmark &bk, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent)
    : QMenu(parent), bm(bk), m_pManager(manager), m_pOwner(owner)
{
    connect(this, &QMenu::aboutToShow, this, &KBookmarkContextMenu::slotAboutToShow);
}

void KBookmarkContextMenu::slotAboutToShow()
{
    addActions();
}

void KBookmarkContextMenu::addActions()
{
    if (bm.isGroup()) {
        addOpenFolderInTabs();
        addBookmark();
        addFolderActions();
    } else {
        addBookmark();
        addBookmarkActions();
    }
}

KBookmarkContextMenu::~KBookmarkContextMenu()
{

}

void KBookmarkContextMenu::addBookmark()
{
    if (m_pOwner && m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)) {
        addAction(QIcon::fromTheme(QStringLiteral("bookmark-new")), tr("Add Bookmark Here"), this, &KBookmarkContextMenu::slotInsert);
    }
}

void KBookmarkContextMenu::addFolderActions()
{
    addAction(tr("Open Folder in Bookmark Editor"), this, &KBookmarkContextMenu::slotEditAt);
    addProperties();
    addSeparator();
    addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete Folder"), this, &KBookmarkContextMenu::slotRemove);
}

void KBookmarkContextMenu::addProperties()
{
    addAction(tr("Properties"), this, &KBookmarkContextMenu::slotProperties);
}

void KBookmarkContextMenu::addBookmarkActions()
{
    addAction(tr("Copy Link Address"), this, &KBookmarkContextMenu::slotCopyLocation);
    addProperties();
    addSeparator();
    addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete Bookmark"), this, &KBookmarkContextMenu::slotRemove);
}

void KBookmarkContextMenu::addOpenFolderInTabs()
{
    if (m_pOwner->supportsTabs()) {
        addAction(QIcon::fromTheme(QStringLiteral("tab-new")), tr("Open Folder in Tabs"), this, &KBookmarkContextMenu::slotOpenFolderInTabs);
    }
}

void KBookmarkContextMenu::slotEditAt()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotEditAt" << m_highlightedAddress;
    m_pManager->slotEditBookmarksAtAddress(bm.address());
}

void KBookmarkContextMenu::slotProperties()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotProperties" << m_highlightedAddress;

    KBookmarkDialog   *dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->editBookmark(bm);
    delete dlg;
}

void KBookmarkContextMenu::slotInsert()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotInsert" << m_highlightedAddress;

    QUrl url = m_pOwner->currentUrl();
    if (url.isEmpty()) {
        QMessageBox::critical(QApplication::activeWindow(), QApplication::applicationName(),
                              tr("Cannot add bookmark with empty URL."));
        return;
    }
    QString title = m_pOwner->currentTitle();
    if (title.isEmpty()) {
        title = url.toDisplayString();
    }

    if (bm.isGroup()) {
        KBookmarkGroup parentBookmark = bm.toGroup();
        Q_ASSERT(!parentBookmark.isNull());
        parentBookmark.addBookmark(title, url, m_pOwner->currentIcon());
        m_pManager->emitChanged(parentBookmark);
    } else {
        KBookmarkGroup parentBookmark = bm.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark(title, m_pOwner->currentUrl(), m_pOwner->currentIcon());
        parentBookmark.moveBookmark(newBookmark, parentBookmark.previous(bm));
        m_pManager->emitChanged(parentBookmark);
    }
}

void KBookmarkContextMenu::slotRemove()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotRemove" << m_highlightedAddress;

    bool folder = bm.isGroup();

    if (QMessageBox::warning(
                QApplication::activeWindow(),
                folder ? tr("Bookmark Folder Deletion")
                : tr("Bookmark Deletion"),
                folder ? tr("Are you sure you wish to remove the bookmark folder\n\"%1\"?").arg(bm.text())
                : tr("Are you sure you wish to remove the bookmark\n\"%1\"?").arg(bm.text()),
                QMessageBox::Yes | QMessageBox::Cancel)
            != QMessageBox::Yes
       ) {
        return;
    }

    KBookmarkGroup parentBookmark = bm.parentGroup();
    parentBookmark.deleteBookmark(bm);
    m_pManager->emitChanged(parentBookmark);
}

void KBookmarkContextMenu::slotCopyLocation()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotCopyLocation" << m_highlightedAddress;

    if (!bm.isGroup()) {
        QMimeData *mimeData = new QMimeData;
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
        mimeData = new QMimeData;
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    }
}

void KBookmarkContextMenu::slotOpenFolderInTabs()
{
    owner()->openFolderinTabs(bookmark().toGroup());
}

KBookmarkManager *KBookmarkContextMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner *KBookmarkContextMenu::owner() const
{
    return m_pOwner;
}

KBookmark KBookmarkContextMenu::bookmark() const
{
    return bm;
}
