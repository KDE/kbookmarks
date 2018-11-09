//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/* This file is part of the KDE libraries
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_opera_p.h"

#include <QFileDialog>
#include "kbookmarks_debug.h"
#include <qtextcodec.h>
#include <QApplication>

#include <qplatformdefs.h>

#include "kbookmarkimporter.h"

void KOperaBookmarkImporter::parseOperaBookmarks()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    Q_ASSERT(codec);
    if (!codec) {
        return;
    }

    QString url, name, type;
    int lineno = 0;
    QTextStream stream(&file);
    stream.setCodec(codec);
    while (! stream.atEnd()) {
        lineno++;
        QString line = stream.readLine().trimmed();

        // first two headers lines contain details about the format
        if (lineno <= 2) {
            if (line.toLower().startsWith(QLatin1String("options:"))) {
                foreach (const QString &ba, line.mid(8).split(QLatin1Char(','))) {
                    const int pos = ba.indexOf(QLatin1Char('='));
                    if (pos < 1) {
                        continue;
                    }
                    const QString key = ba.left(pos).trimmed().toLower();
                    const QString value = ba.mid(pos + 1).trimmed();
                }
            }
            continue;
        }

        // at least up till version<=3 the following is valid
        if (line.isEmpty()) {
            // end of data block
            if (type.isNull()) {
                continue;
            } else if (type == QLatin1String("URL")) {
                emit newBookmark(name, url, QLatin1String(""));
            } else if (type == QLatin1String("FOLDER")) {
                emit newFolder(name, false, QLatin1String(""));
            }

            type.clear();
            name.clear();
            url.clear();
        } else if (line == QLatin1String("-")) {
            // end of folder
            emit endFolder();
        } else {
            // data block line
            QString tag;
            if (tag = QStringLiteral("#"), line.startsWith(tag)) {
                type = line.remove(0, tag.length());
            } else if (tag = QStringLiteral("NAME="), line.startsWith(tag)) {
                name = line.remove(0, tag.length());
            } else if (tag = QStringLiteral("URL="), line.startsWith(tag)) {
                url = line.remove(0, tag.length());
            }
        }
    }
}

QString KOperaBookmarkImporter::operaBookmarksFile()
{
    static KOperaBookmarkImporterImpl *p = nullptr;
    if (!p) {
        p = new KOperaBookmarkImporterImpl;
    }
    return p->findDefaultLocation();
}

void KOperaBookmarkImporterImpl::parse()
{
    KOperaBookmarkImporter importer(m_fileName);
    setupSignalForwards(&importer, this);
    importer.parseOperaBookmarks();
}

QString KOperaBookmarkImporterImpl::findDefaultLocation(bool saving) const
{
    const QString operaHomePath = QDir::homePath() + QLatin1String("/.opera");
    return saving ? QFileDialog::getSaveFileName(QApplication::activeWindow(), QString(),
            operaHomePath,
            tr("Opera Bookmark Files (*.adr)"))
           : QFileDialog::getOpenFileName(QApplication::activeWindow(), QString(),
                                          operaHomePath,
                                          tr("*.adr|Opera Bookmark Files (*.adr)"));
}

/////////////////////////////////////////////////

class OperaExporter : private KBookmarkGroupTraverser
{
public:
    OperaExporter();
    QString generate(const KBookmarkGroup &grp)
    {
        traverse(grp);
        return m_string;
    }
private:
    void visit(const KBookmark &) override;
    void visitEnter(const KBookmarkGroup &) override;
    void visitLeave(const KBookmarkGroup &) override;
private:
    QString m_string;
    QTextStream m_out;
};

OperaExporter::OperaExporter() : m_out(&m_string, QIODevice::WriteOnly)
{
    m_out << "Opera Hotlist version 2.0" << endl;
    m_out << "Options: encoding = utf8, version=3" << endl;
}

void OperaExporter::visit(const KBookmark &bk)
{
    // qCDebug(KBOOKMARKS_LOG) << "visit(" << bk.text() << ")";
    m_out << "#URL" << endl;
    m_out << "\tNAME=" << bk.fullText() << endl;
    m_out << "\tURL=" << bk.url().toString().toUtf8() << endl;
    m_out << endl;
}

void OperaExporter::visitEnter(const KBookmarkGroup &grp)
{
    // qCDebug(KBOOKMARKS_LOG) << "visitEnter(" << grp.text() << ")";
    m_out << "#FOLDER" << endl;
    m_out << "\tNAME=" << grp.fullText() << endl;
    m_out << endl;
}

void OperaExporter::visitLeave(const KBookmarkGroup &)
{
    // qCDebug(KBOOKMARKS_LOG) << "visitLeave()";
    m_out << "-" << endl;
    m_out << endl;
}

void KOperaBookmarkExporterImpl::write(const KBookmarkGroup &parent)
{
    OperaExporter exporter;
    QString content = exporter.generate(parent);
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCCritical(KBOOKMARKS_LOG) << "Can't write to file " << m_fileName << endl;
        return;
    }
    QTextStream fstream(&file);
    fstream.setCodec(QTextCodec::codecForName("UTF-8"));
    fstream << content;
}

#include "moc_kbookmarkimporter_opera_p.cpp"
