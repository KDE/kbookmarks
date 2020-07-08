//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/* This file is part of the KDE libraries
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter.h"

#include "kbookmarks_debug.h"
#include <KCharsets>
#include <QTextCodec>

#include <qplatformdefs.h>
#include <stddef.h>
#include <assert.h>


#include "kbookmarkmanager.h"

#include "kbookmarkimporter_ns.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"

void KXBELBookmarkImporterImpl::parse()
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::parse()";
    KBookmarkManager *manager = KBookmarkManager::managerForFile(m_fileName, QString());
    KBookmarkGroup root = manager->root();
    traverse(root);
    // FIXME delete it!
    // delete manager;
}

void KXBELBookmarkImporterImpl::visit(const KBookmark &bk)
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::visit";
    if (bk.isSeparator()) {
        emit newSeparator();
    } else {
        emit newBookmark(bk.fullText(), bk.url().toString(), QLatin1String(""));
    }
}

void KXBELBookmarkImporterImpl::visitEnter(const KBookmarkGroup &grp)
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::visitEnter";
    emit newFolder(grp.fullText(), false, QLatin1String(""));
}

void KXBELBookmarkImporterImpl::visitLeave(const KBookmarkGroup &)
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::visitLeave";
    emit endFolder();
}

void KBookmarkImporterBase::setupSignalForwards(QObject *src, QObject *dst)
{
    connect(src, SIGNAL(newBookmark(QString,QString,QString)),
            dst, SIGNAL(newBookmark(QString,QString,QString)));
    connect(src, SIGNAL(newFolder(QString,bool,QString)),
            dst, SIGNAL(newFolder(QString,bool,QString)));
    connect(src, SIGNAL(newSeparator()),
            dst, SIGNAL(newSeparator()));
    connect(src, SIGNAL(endFolder()),
            dst, SIGNAL(endFolder()));
}

KBookmarkImporterBase *KBookmarkImporterBase::factory(const QString &type)
{
    if (type == QLatin1String("netscape")) {
        return new KNSBookmarkImporterImpl;
    } else if (type == QLatin1String("mozilla")) {
        return new KMozillaBookmarkImporterImpl;
    } else if (type == QLatin1String("xbel")) {
        return new KXBELBookmarkImporterImpl;
    } else if (type == QLatin1String("ie")) {
        return new KIEBookmarkImporterImpl;
    } else if (type == QLatin1String("opera")) {
        return new KOperaBookmarkImporterImpl;
    } else {
        return nullptr;
    }
}

#include "moc_kbookmarkimporter.cpp"
