/***************************************************************************
 *   Copyright (C) 2019 by David Hallas <david@davidhallas.dk>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include <kbookmarkmenu.h>
#include <kbookmarkowner.h>
#include <kbookmarkmanager.h>
#include <KXmlGui/kactioncollection.h>

#include <QDebug>
#include <QMenu>
#include <QStandardPaths>
#include <QStringLiteral>
#include <QTest>

class TestKBookmarkOwner;

class KBookmarkMenuTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void tabsOpen_data();
    void tabsOpen();
    void tabsOpenChanges();
};

static bool hasBookmarkEditorInstalled()
{
    static bool isInstalled = !QStandardPaths::findExecutable(QStringLiteral("keditbookmarks")).isEmpty();
    return isInstalled;
}

static int actionCountWithoutBookmarkTabsAsFolder()
{
    return hasBookmarkEditorInstalled() ? 3 : 2;
}

static int actionCountWithBookmarkTabsAsFolder()
{
    return hasBookmarkEditorInstalled() ? 4 : 3;
}

class TestKBookmarkOwner final : public KBookmarkOwner
{
public:
    explicit TestKBookmarkOwner(bool supportsTabs_);
    void openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) override;
    bool supportsTabs() const override;
private:
    const bool m_supportsTabs;
};

TestKBookmarkOwner::TestKBookmarkOwner(bool supportsTabs_) :
    m_supportsTabs(supportsTabs_)
{
}

void TestKBookmarkOwner::openBookmark(const KBookmark&, Qt::MouseButtons, Qt::KeyboardModifiers)
{
}

bool TestKBookmarkOwner::supportsTabs() const
{
    return m_supportsTabs;
}

#define VERIFY_MENU_WITHOUT_BOOKMARK_TABS_AS_FOLDER() \
    do { \
    QCOMPARE(menu->actions().count(), actionCountWithoutBookmarkTabsAsFolder()); \
        QCOMPARE(menu->actions().at(0)->text(), QStringLiteral("&Add Bookmark")); \
        QCOMPARE(menu->actions().at(1)->text(), QStringLiteral("New Bookmark Folder...")); \
        if (hasBookmarkEditorInstalled()) { \
            QCOMPARE(menu->actions().at(2)->text(), QStringLiteral("&Edit Bookmarks...")); \
        } \
    } while (false)

#define VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER() \
    do { \
        QCOMPARE(menu->actions().count(), actionCountWithBookmarkTabsAsFolder()); \
        QCOMPARE(menu->actions().at(0)->text(), QStringLiteral("&Add Bookmark")); \
        QCOMPARE(menu->actions().at(1)->text(), QStringLiteral("Bookmark Tabs as Folder...")); \
        QCOMPARE(menu->actions().at(2)->text(), QStringLiteral("New Bookmark Folder...")); \
        if (hasBookmarkEditorInstalled()) { \
            QCOMPARE(menu->actions().at(3)->text(), QStringLiteral("&Edit Bookmarks...")); \
        } \
    } while (false)

void KBookmarkMenuTest::tabsOpen_data()
{
    QTest::addColumn<bool>("supportsTabs");
    QTest::addColumn<int>("numberOfOpenTabs");

    const bool TabsNotSupported(false);
    QTest::newRow("Tabs not supported - 0 tabs open") << TabsNotSupported << 0;
    QTest::newRow("Tabs not supported - 1 tabs open") << TabsNotSupported << 1;
    QTest::newRow("Tabs not supported - 2 tabs open") << TabsNotSupported << 2;
    const bool TabsSupported(true);
    QTest::newRow("Tabs supported - 0 tabs open") << TabsSupported << 0;
    QTest::newRow("Tabs supported - 1 tabs open") << TabsSupported << 1;
    QTest::newRow("Tabs supported - 2 tabs open") << TabsSupported << 2;
}

void KBookmarkMenuTest::tabsOpen()
{
    QFETCH(bool, supportsTabs);
    QFETCH(int, numberOfOpenTabs);
    auto manager = KBookmarkManager::createTempManager();
    QScopedPointer<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(supportsTabs));
    QMenu* menu = new QMenu;
    QScopedPointer<KActionCollection> actionCollection(new KActionCollection(nullptr, QString()));
    QScopedPointer<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.data(), menu, actionCollection.data()));
    testObject->setNumberOfOpenTabs(numberOfOpenTabs);
    testObject->ensureUpToDate();
    if (supportsTabs && numberOfOpenTabs > 1) {
        VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER();
    } else {
        VERIFY_MENU_WITHOUT_BOOKMARK_TABS_AS_FOLDER();
    }
}

void KBookmarkMenuTest::tabsOpenChanges()
{
    auto manager = KBookmarkManager::createTempManager();
    QScopedPointer<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(true));
    QMenu* menu = new QMenu;
    QScopedPointer<KActionCollection> actionCollection(new KActionCollection(nullptr, QString()));
    QScopedPointer<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.data(), menu, actionCollection.data()));
    testObject->ensureUpToDate();
    // If the number of open tabs has not been set it will default to 2
    const int DefaultNumberOfOpenTabs = 2;
    QCOMPARE(testObject->numberOfOpenTabs(), DefaultNumberOfOpenTabs);
    testObject->ensureUpToDate();
    VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER();
    testObject->setNumberOfOpenTabs(1);
    testObject->ensureUpToDate();
    VERIFY_MENU_WITHOUT_BOOKMARK_TABS_AS_FOLDER();
    testObject->setNumberOfOpenTabs(2);
    testObject->ensureUpToDate();
    VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER();
}

QTEST_MAIN(KBookmarkMenuTest)

#include "kbookmarkmenutest.moc"
