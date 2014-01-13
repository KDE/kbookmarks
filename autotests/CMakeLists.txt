remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

include(ECMMarkAsTest)

add_executable(kbookmarktest kbookmarktest.cpp)
ecm_mark_as_test(kbookmarktest)
add_test(kbookmark-kbookmarktest kbookmarktest)
target_link_libraries(kbookmarktest KF5::Bookmarks Qt5::Test)

