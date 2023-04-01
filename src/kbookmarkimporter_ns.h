//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __kbookmarkimporter_ns_h
#define __kbookmarkimporter_ns_h

#include "kbookmarkexporter.h"
#include "kbookmarkimporter.h"

/**
 * A class for importing NS bookmarks
 * utf8 defaults to off
 */
class KBOOKMARKS_EXPORT KNSBookmarkImporterImpl : public KBookmarkImporterBase
{
    Q_OBJECT // For QObject::tr
        public : KNSBookmarkImporterImpl()
        : m_utf8(false)
    {
    }
    void setUtf8(bool utf8)
    {
        m_utf8 = utf8;
    }
    void parse() override;
    QString findDefaultLocation(bool forSaving = false) const override;

private:
    KBOOKMARKS_NO_EXPORT QString toUnicode(const QByteArray &data) const;

private:
    bool m_utf8;
    class KNSBookmarkImporterImplPrivate *d;
};

/**
 * A class for importing Mozilla bookmarks
 * utf8 defaults to on
 */
class KBOOKMARKS_EXPORT KMozillaBookmarkImporterImpl : public KNSBookmarkImporterImpl
{
    Q_OBJECT
public:
    KMozillaBookmarkImporterImpl()
    {
        setUtf8(true);
    }

private:
    class KMozillaBookmarkImporterImplPrivate *d;
};

class KBOOKMARKS_EXPORT KNSBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KNSBookmarkExporterImpl(KBookmarkManager *mgr, const QString &fileName)
        : KBookmarkExporterBase(mgr, fileName)
    {
        ;
    }
    ~KNSBookmarkExporterImpl() override
    {
    }
    void write(const KBookmarkGroup &parent) override;
    void setUtf8(bool);

protected:
    QString folderAsString(const KBookmarkGroup &parent) const;

private:
    bool m_utf8;
    class KNSBookmarkExporterImplPrivate *d;
};

#endif
