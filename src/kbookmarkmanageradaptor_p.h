//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KBOOKMARKMANAGERADAPTOR_H
#define KBOOKMARKMANAGERADAPTOR_H

#include <QDBusAbstractAdaptor>

class KBookmarkManager;

class KBookmarkManagerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KIO.KBookmarkManager")
public:
    explicit KBookmarkManagerAdaptor(KBookmarkManager *parent);

public Q_SLOTS:
    // Not used by KDE, but useful for scripts, after changing the bookmarks.xml file.
    void notifyCompleteChange();

Q_SIGNALS:
    void bookmarkCompleteChange(const QString &caller);

    void bookmarksChanged(const QString &groupAddress);

    void bookmarkConfigChanged();
};

#endif
