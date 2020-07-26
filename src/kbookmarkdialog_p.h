//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Jignesh Kakadiya <jigneshhk1992@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef __kbookmarkdialog_p_h__
#define __kbookmarkdialog_p_h__

#include "kbookmark.h"
#include <QDialog>

class KBookmarkDialog;
class KBookmarkManager;
class QDialogButtonBox;
class QLabel;
class QTreeWidget;
class QLineEdit;
class QTreeWidgetItem;

class KBookmarkDialogPrivate
{
public:
    explicit KBookmarkDialogPrivate(KBookmarkDialog *q);
    ~KBookmarkDialogPrivate();

    typedef enum { NewFolder, NewBookmark, EditBookmark, NewMultipleBookmarks, SelectFolder } BookmarkDialogMode;

    void initLayout();
    void initLayoutPrivate();
    // selects the specified bookmark in the folder tree
    void setParentBookmark(const KBookmark &bm);
    KBookmarkGroup parentBookmark();
    void fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group, const KBookmarkGroup &selectGroup = KBookmarkGroup());

    KBookmarkDialog *q;
    BookmarkDialogMode mode;
    QDialogButtonBox *buttonBox;
    QLineEdit *url;
    QLineEdit *title;
    QLineEdit *comment;
    QLabel *titleLabel;
    QLabel *urlLabel;
    QLabel *commentLabel;
    QString icon;
    QTreeWidget *folderTree;
    KBookmarkManager *mgr;
    KBookmark bm;
    QList<KBookmarkOwner::FutureBookmark> list;
    bool layout;
};

#endif
