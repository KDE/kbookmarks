//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkimporter.h"

#include "kbookmarks_debug.h"

#include <assert.h>
#include <qplatformdefs.h>
#include <stddef.h>

#include "kbookmarkmanager.h"

#include "kbookmarkimporter_ie.h"
#include "kbookmarkimporter_ns.h"
#include "kbookmarkimporter_opera.h"

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
        Q_EMIT newSeparator();
    } else {
        Q_EMIT newBookmark(bk.fullText(), bk.url().toString(), QLatin1String(""));
    }
}

void KXBELBookmarkImporterImpl::visitEnter(const KBookmarkGroup &grp)
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::visitEnter";
    Q_EMIT newFolder(grp.fullText(), false, QLatin1String(""));
}

void KXBELBookmarkImporterImpl::visitLeave(const KBookmarkGroup &)
{
    // qCDebug(KBOOKMARKS_LOG) << "KXBELBookmarkImporterImpl::visitLeave";
    Q_EMIT endFolder();
}

void KBookmarkImporterBase::setupSignalForwards(QObject *src, QObject *dst)
{
    // clang-format off
    connect(src, SIGNAL(newBookmark(QString,QString,QString)), dst, SIGNAL(newBookmark(QString,QString,QString)));
    connect(src, SIGNAL(newFolder(QString,bool,QString)), dst, SIGNAL(newFolder(QString,bool,QString)));
    // clang-format on
    connect(src, SIGNAL(newSeparator()), dst, SIGNAL(newSeparator()));
    connect(src, SIGNAL(endFolder()), dst, SIGNAL(endFolder()));
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
