/*
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>
#include <kbookmarkowner.h>
#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
#include <KActionCollection>
#endif

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
#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
    void tabsOpenActionCollection_data();
    void tabsOpenActionCollection();
    void tabsOpenChangesActionCollection();
#endif
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

TestKBookmarkOwner::TestKBookmarkOwner(bool supportsTabs_)
    : m_supportsTabs(supportsTabs_)
{
}

void TestKBookmarkOwner::openBookmark(const KBookmark &, Qt::MouseButtons, Qt::KeyboardModifiers)
{
}

bool TestKBookmarkOwner::supportsTabs() const
{
    return m_supportsTabs;
}

#define VERIFY_MENU_WITHOUT_BOOKMARK_TABS_AS_FOLDER()                                                                                                          \
    do {                                                                                                                                                       \
        QCOMPARE(menu->actions().count(), actionCountWithoutBookmarkTabsAsFolder());                                                                           \
        QCOMPARE(menu->actions().at(0)->text(), QStringLiteral("&Add Bookmark"));                                                                              \
        QCOMPARE(menu->actions().at(1)->text(), QStringLiteral("New Bookmark Folder..."));                                                                     \
        if (hasBookmarkEditorInstalled()) {                                                                                                                    \
            QCOMPARE(menu->actions().at(2)->text(), QStringLiteral("&Edit Bookmarks..."));                                                                     \
        }                                                                                                                                                      \
    } while (false)

#define VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER()                                                                                                             \
    do {                                                                                                                                                       \
        QCOMPARE(menu->actions().count(), actionCountWithBookmarkTabsAsFolder());                                                                              \
        QCOMPARE(menu->actions().at(0)->text(), QStringLiteral("&Add Bookmark"));                                                                              \
        QCOMPARE(menu->actions().at(1)->text(), QStringLiteral("Bookmark Tabs as Folder..."));                                                                 \
        QCOMPARE(menu->actions().at(2)->text(), QStringLiteral("New Bookmark Folder..."));                                                                     \
        if (hasBookmarkEditorInstalled()) {                                                                                                                    \
            QCOMPARE(menu->actions().at(3)->text(), QStringLiteral("&Edit Bookmarks..."));                                                                     \
        }                                                                                                                                                      \
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
    std::unique_ptr<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(supportsTabs));
    QMenu *menu = new QMenu;
    std::unique_ptr<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.get(), menu));
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
    std::unique_ptr<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(true));
    QMenu *menu = new QMenu;
    std::unique_ptr<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.get(), menu));
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

#if KBOOKMARKS_BUILD_DEPRECATED_SINCE(5, 69)
void KBookmarkMenuTest::tabsOpenActionCollection_data()
{
    tabsOpen_data();
}

void KBookmarkMenuTest::tabsOpenActionCollection()
{
    QFETCH(bool, supportsTabs);
    QFETCH(int, numberOfOpenTabs);
    auto manager = KBookmarkManager::createTempManager();
    std::unique_ptr<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(supportsTabs));
    QMenu *menu = new QMenu;
    std::unique_ptr<KActionCollection> actionCollection(new KActionCollection(nullptr, QString()));
    std::unique_ptr<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.get(), menu, actionCollection.get()));
    testObject->setNumberOfOpenTabs(numberOfOpenTabs);
    testObject->ensureUpToDate();
    if (supportsTabs && numberOfOpenTabs > 1) {
        VERIFY_MENU_WITH_BOOKMARK_TABS_AS_FOLDER();
    } else {
        VERIFY_MENU_WITHOUT_BOOKMARK_TABS_AS_FOLDER();
    }
}

void KBookmarkMenuTest::tabsOpenChangesActionCollection()
{
    auto manager = KBookmarkManager::createTempManager();
    std::unique_ptr<TestKBookmarkOwner> bookmarkOwner(new TestKBookmarkOwner(true));
    QMenu *menu = new QMenu;
    std::unique_ptr<KActionCollection> actionCollection(new KActionCollection(nullptr, QString()));
    std::unique_ptr<KBookmarkMenu> testObject(new KBookmarkMenu(manager, bookmarkOwner.get(), menu, actionCollection.get()));
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
#endif

QTEST_MAIN(KBookmarkMenuTest)

#include "kbookmarkmenutest.moc"
