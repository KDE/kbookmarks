remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt${QT_MAJOR_VERSION}::Test)
    message(STATUS "Qt${QT_MAJOR_VERSION}Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_test(kbookmarktest.cpp TEST_NAME kbookmarktest LINK_LIBRARIES KF5::Bookmarks Qt${QT_MAJOR_VERSION}::Test)
ecm_add_test(kbookmarkmenutest.cpp TEST_NAME kbookmarkmenutest LINK_LIBRARIES KF5::Bookmarks Qt${QT_MAJOR_VERSION}::Test)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.69.0)
    target_link_libraries(kbookmarkmenutest KF5::XmlGui)
endif()
