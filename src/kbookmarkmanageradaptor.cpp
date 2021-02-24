//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkmanager.h"
#include "kbookmarkmanageradaptor_p.h"

KBookmarkManagerAdaptor::KBookmarkManagerAdaptor(KBookmarkManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

void KBookmarkManagerAdaptor::notifyCompleteChange()
{
    static_cast<KBookmarkManager *>(parent())->notifyCompleteChange(QString());
}

#include "moc_kbookmarkmanageradaptor_p.cpp"
