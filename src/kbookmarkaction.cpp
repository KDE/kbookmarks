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

#include "kbookmarkaction.h"
#include "kbookmarkowner.h"
#include <QDesktopServices>
#include <QApplication>

KBookmarkAction::KBookmarkAction(const KBookmark &bk, KBookmarkOwner *owner, QObject *parent)
    : QAction(bk.text().replace('&', "&&"), parent),
      KBookmarkActionInterface(bk),
      m_pOwner(owner)
{
    setIcon(QIcon::fromTheme(bookmark().icon()));
    setIconText(text());
    setToolTip(bookmark().url().toDisplayString(QUrl::PreferLocalFile));
    setStatusTip(toolTip());
    setWhatsThis(toolTip());
    const QString description = bk.description();
    if (!description.isEmpty()) {
        setToolTip(description);
    }
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotTriggered()));
}

KBookmarkAction::~KBookmarkAction()
{
}

void KBookmarkAction::slotTriggered()
{
    slotSelected(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

void KBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    if (!m_pOwner) {
        QDesktopServices::openUrl(bookmark().url());
    } else {
        m_pOwner->openBookmark(bookmark(), mb, km);
    }
}

