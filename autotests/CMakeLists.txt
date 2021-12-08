remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt${QT_MAJOR_VERSION}::Test)
    message(STATUS "Qt${QT_MAJOR_VERSION}Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_test(kbookmarktest.cpp TEST_NAME kbookmarktest LINK_LIBRARIES KF5::Bookmarks Qt${QT_MAJOR_VERSION}::Test)
ecm_add_test(kbookmarkmenutest.cpp TEST_NAME kbookmarkmenutest LINK_LIBRARIES KF5::Bookmarks KF5::XmlGui Qt${QT_MAJOR_VERSION}::Test)
