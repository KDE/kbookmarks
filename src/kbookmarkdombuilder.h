/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kbookmarkdombuilder_h
#define __kbookmarkdombuilder_h

#include <kbookmark.h>

#include <QObject>
#include <QStack>

/**
 * @class KBookmarkDomBuilder kbookmarkdombuilder.h KBookmarkDomBuilder
 *
 * A class used for constructing bookmark objects during import.
 */
class KBOOKMARKS_EXPORT KBookmarkDomBuilder : public QObject
{
    Q_OBJECT
public:
    KBookmarkDomBuilder(const KBookmarkGroup &group, KBookmarkManager *);
    ~KBookmarkDomBuilder() override;
    void connectImporter(const QObject *);
protected Q_SLOTS:
    void newBookmark(const QString &text, const QString &url, const QString &additionalInfo);
    void newFolder(const QString &text, bool open, const QString &additionalInfo);
    void newSeparator();
    void endFolder();

private:
    QStack<KBookmarkGroup> m_stack;
    QList<KBookmarkGroup> m_list;
    KBookmarkManager *m_manager;
    class KBookmarkDomBuilderPrivate *p;
};

#endif
