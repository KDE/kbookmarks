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

#include "kbookmarkactionmenu.h"

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, QObject *parent)
    : KActionMenu(QIcon::fromTheme(bm.icon()), bm.text().replace('&', "&&"), parent),
      KBookmarkActionInterface(bm)
{
    setToolTip(bm.description());
    setIconText(text());
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, const QString &text, QObject *parent)
    : KActionMenu(text, parent),
      KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

