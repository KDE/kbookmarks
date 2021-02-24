//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __kbookmarkimporter_ie_h
#define __kbookmarkimporter_ie_h

#include <kbookmarkexporter.h>
#include <kbookmarkimporter.h>

/**
 * A class for importing IE bookmarks
 */
class KBOOKMARKS_EXPORT KIEBookmarkImporterImpl : public KBookmarkImporterBase
{
    Q_OBJECT
public:
    KIEBookmarkImporterImpl()
    {
    }
    void parse() override;
    QString findDefaultLocation(bool forSaving = false) const override;

private:
    class KIEBookmarkImporterImplPrivate *d;
};

class KBOOKMARKS_EXPORT KIEBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KIEBookmarkExporterImpl(KBookmarkManager *mgr, const QString &path)
        : KBookmarkExporterBase(mgr, path)
    {
        ;
    }
    ~KIEBookmarkExporterImpl() override
    {
    }
    void write(const KBookmarkGroup &) override;

private:
    class KIEBookmarkExporterImplPrivate *d;
};

#endif
