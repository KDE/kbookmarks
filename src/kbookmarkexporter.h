//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1996-1998 Martin R. Jones <mjones@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __kbookmarkexporter_h
#define __kbookmarkexporter_h

#include <kbookmark.h>

/**
 * A class for exporting bookmarks
 */
class KBOOKMARKS_EXPORT KBookmarkExporterBase
{
public:
    KBookmarkExporterBase(KBookmarkManager *mgr, const QString &fileName)
        : m_fileName(fileName)
        , m_pManager(mgr)
    {
    }
    virtual ~KBookmarkExporterBase()
    {
    }
    virtual void write(const KBookmarkGroup &) = 0;

protected:
    QString m_fileName;
    KBookmarkManager *m_pManager;

private:
    class KBookmarkExporterBasePrivate *d;
};

#endif
