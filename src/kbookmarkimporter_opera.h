/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __kbookmarkimporter_opera_h
#define __kbookmarkimporter_opera_h

#include <kbookmarkexporter.h>
#include <kbookmarkimporter.h>

/**
 * A class for importing Opera bookmarks
 */
class KBOOKMARKS_EXPORT KOperaBookmarkImporterImpl : public KBookmarkImporterBase
{
    Q_OBJECT // For QObject::tr
        public : KOperaBookmarkImporterImpl()
    {
    }
    void parse() override;
    QString findDefaultLocation(bool forSaving = false) const override;

private:
    class KOperaBookmarkImporterImplPrivate *d;
};

class KBOOKMARKS_EXPORT KOperaBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KOperaBookmarkExporterImpl(KBookmarkManager *mgr, const QString &filename)
        : KBookmarkExporterBase(mgr, filename)
    {
        ;
    }
    ~KOperaBookmarkExporterImpl() override
    {
    }
    void write(const KBookmarkGroup &parent) override;

private:
    class KOperaBookmarkExporterImplPrivate *d;
};

#endif
