/*
    SPDX-FileCopyrightText: 2013 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QApplication>
#include <QPushButton>

#include <kbookmarkdialog.h>
#include <kbookmarkmanager.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KBookmarkDialog dlg(KBookmarkManager::userBookmarksManager());
    dlg.addBookmark(QStringLiteral("KDE"), QUrl(QStringLiteral("http://www.kde.org")), QStringLiteral("www")); // calls exec()
    return 0;
}
