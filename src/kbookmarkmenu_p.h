//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <QObject>

#include <QTreeWidget>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class KBookmark;
class KBookmarkGroup;
class KBookmarkOwner;
class KBookmarkMenu;

#define KEDITBOOKMARKS_BINARY "keditbookmarks"

class KBookmarkTreeItem : public QTreeWidgetItem
{
public:
    explicit KBookmarkTreeItem(QTreeWidget *tree);
    KBookmarkTreeItem(QTreeWidgetItem *parent, QTreeWidget *tree, const KBookmarkGroup &bk);
    ~KBookmarkTreeItem();
    QString address();
private:
    QString m_address;
};

class KBookmarkSettings
{
public:
    bool m_advancedaddbookmark;
    bool m_contextmenu;
    static KBookmarkSettings *s_self;
    static void readSettings();
    static KBookmarkSettings *self();
};

#endif
