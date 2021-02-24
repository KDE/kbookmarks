//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1996-1998 Martin R. Jones <mjones@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkimporter_ns.h"
#include "kbookmarkmanager.h"
#include "kbookmarks_debug.h"
#include <KCharsets>
#include <QFileDialog>
#include <QMessageBox>

#include <QApplication>
#include <QTextCodec>

void KNSBookmarkImporterImpl::parse()
{
    QFile f(m_fileName);
    QTextCodec *codec = m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale();
    Q_ASSERT(codec);
    if (!codec) {
        return;
    }

    if (f.open(QIODevice::ReadOnly)) {
        static const int g_lineLimit = 16 * 1024;
        QByteArray s(g_lineLimit, 0);
        // skip header
        while (f.readLine(s.data(), g_lineLimit) >= 1 && !s.contains("<DL>")) {
            ;
        }

        while (int size = f.readLine(s.data(), g_lineLimit) >= 1) {
            if (size == g_lineLimit) { // Gosh, this line is longer than g_lineLimit. Skipping.
                qCWarning(KBOOKMARKS_LOG) << "Netscape bookmarks contain a line longer than " << g_lineLimit << ". Skipping.";
                continue;
            }
            QByteArray t = s.trimmed();

            if (t.left(4).toUpper() == "<HR>") {
                Q_EMIT newSeparator();
                t = t.mid(4).trimmed();
                if (t.isEmpty()) {
                    continue;
                }
            }

            if (t.left(12).toUpper() == "<DT><A HREF=" || t.left(16).toUpper() == "<DT><H3><A HREF=") {
                int firstQuotes = t.indexOf('"') + 1;
                int secondQuotes = t.indexOf('"', firstQuotes);
                if (firstQuotes != -1 && secondQuotes != -1) {
                    QByteArray link = t.mid(firstQuotes, secondQuotes - firstQuotes);
                    int endTag = t.indexOf('>', secondQuotes + 1);

                    int closeTag = t.indexOf('<', endTag + 1);

                    QByteArray name = t.mid(endTag + 1, closeTag - endTag - 1);
                    QString qname = KCharsets::resolveEntities(codec->toUnicode(name));

                    Q_EMIT newBookmark(qname, codec->toUnicode(link), QString());
                }
            } else if (t.left(7).toUpper() == "<DT><H3") {
                int endTag = t.indexOf('>', 7);
                QByteArray name = t.mid(endTag + 1);
                name = name.left(name.indexOf('<'));
                QString qname = KCharsets::resolveEntities(codec->toUnicode(name));
                QByteArray additionalInfo = t.mid(8, endTag - 8);
                bool folded = (additionalInfo.left(6) == "FOLDED");
                if (folded) {
                    additionalInfo.remove(0, 7);
                }

                Q_EMIT newFolder(qname, !folded, QString());
            } else if (t.left(8).toUpper() == "</DL><P>") {
                Q_EMIT endFolder();
            }
        }

        f.close();
    }
}

QString KNSBookmarkImporterImpl::findDefaultLocation(bool forSaving) const
{
    if (m_utf8) {
        const QString mozillaHomePath = QDir::homePath() + QLatin1String("/.mozilla");
        if (forSaving)
            return QFileDialog::getSaveFileName(QApplication::activeWindow(), QString(), mozillaHomePath, tr("HTML Files (*.html)"));
        else
            return QFileDialog::getOpenFileName(QApplication::activeWindow(), QString(), mozillaHomePath, tr("*.html|HTML Files (*.html)"));
    } else {
        return QDir::homePath() + QLatin1String("/.netscape/bookmarks.html");
    }
}

////////////////////////////////////////////////////////////////

void KNSBookmarkExporterImpl::setUtf8(bool utf8)
{
    m_utf8 = utf8;
}

void KNSBookmarkExporterImpl::write(const KBookmarkGroup &parent)
{
    if (!QFile::exists(m_fileName)) {
        QString errorMsg = KNSBookmarkImporterImpl::tr(
                               "Could not find %1. Netscape is probably not installed. "
                               "Aborting the export.")
                               .arg(m_fileName);
        QMessageBox::critical(nullptr, KNSBookmarkImporterImpl::tr("Netscape not found"), errorMsg);
        return;
    }
    if (QFile::exists(m_fileName)) {
        (void)QFile::rename(m_fileName, m_fileName + QLatin1String(".beforekde"));
    }

    QFile file(m_fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        qCCritical(KBOOKMARKS_LOG) << "Can't write to file " << m_fileName;
        return;
    }

    QTextStream fstream(&file);
    fstream.setCodec(m_utf8 ? QTextCodec::codecForName("UTF-8") : QTextCodec::codecForLocale());

    QString charset = m_utf8 ? QStringLiteral("UTF-8") : QString::fromLatin1(QTextCodec::codecForLocale()->name()).toUpper();

    fstream << "<!DOCTYPE NETSCAPE-Bookmark-file-1>\n"
            << KNSBookmarkImporterImpl::tr("<!-- This file was generated by Konqueror -->") << "\n"
            << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=" << charset << "\">\n"
            << "<TITLE>" << KNSBookmarkImporterImpl::tr("Bookmarks") << "</TITLE>\n"
            << "<H1>" << KNSBookmarkImporterImpl::tr("Bookmarks") << "</H1>\n"
            << "<DL><p>\n"
            << folderAsString(parent) << "</DL><P>\n";
}

QString KNSBookmarkExporterImpl::folderAsString(const KBookmarkGroup &parent) const
{
    QString str;
    QTextStream fstream(&str, QIODevice::WriteOnly);

    for (KBookmark bk = parent.first(); !bk.isNull(); bk = parent.next(bk)) {
        if (bk.isSeparator()) {
            fstream << "<HR>\n";
            fstream.flush();
            continue;
        }

        QString text = bk.fullText().toHtmlEscaped();

        if (bk.isGroup()) {
            fstream << "<DT><H3 " << (!bk.toGroup().isOpen() ? "FOLDED " : "") << bk.internalElement().attribute(QStringLiteral("netscapeinfo")) << ">" << text
                    << "</H3>\n"
                    << "<DL><P>\n"
                    << folderAsString(bk.toGroup()) << "</DL><P>\n";
            fstream.flush();
            continue;

        } else {
            // note - netscape seems to use local8bit for url...
            fstream << "<DT><A HREF=\"" << bk.url().toString() << "\"" << bk.internalElement().attribute(QStringLiteral("netscapeinfo")) << ">" << text
                    << "</A>\n";
            fstream.flush();
            continue;
        }
    }

    return str;
}

////
