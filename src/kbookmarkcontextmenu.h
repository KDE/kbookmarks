/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOOKMARKCONTEXTMENU_H
#define KBOOKMARKCONTEXTMENU_H

#include <QMenu>

#include "kbookmark.h"

class KBookmarkManager;
class KBookmarkOwner;

/**
 * @class KBookmarkContextMenu kbookmarkcontextmenu.h KBookmarkContextMenu
 *
 * A context menu for a bookmark.
 */
class KBOOKMARKS_EXPORT KBookmarkContextMenu : public QMenu
{
    Q_OBJECT

public:
    KBookmarkContextMenu(const KBookmark &bm, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent = nullptr);
    ~KBookmarkContextMenu() override;
    virtual void addActions();

public Q_SLOTS:
    void slotEditAt();
    void slotProperties();
    void slotInsert();
    void slotRemove();
    void slotCopyLocation();
    void slotOpenFolderInTabs();

protected:
    void addBookmark();
    void addFolderActions();
    void addProperties();
    void addBookmarkActions();
    void addOpenFolderInTabs();

    KBookmarkManager *manager() const;
    KBookmarkOwner *owner() const;
    KBookmark bookmark() const;

private Q_SLOTS:
    KBOOKMARKS_NO_EXPORT void slotAboutToShow();

private:
    const KBookmark bm;
    KBookmarkManager *const m_pManager;
    KBookmarkOwner *const m_pOwner;
};

#endif
