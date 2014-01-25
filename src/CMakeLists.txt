# TODO
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kbookmarks_SRCS
  kbookmark.cpp
  kbookmarkaction.cpp
  kbookmarkactioninterface.cpp
  kbookmarkactionmenu.cpp
  kbookmarkcontextmenu.cpp
  kbookmarkimporter.cpp
  kbookmarkmanager.cpp
  kbookmarkmanageradaptor.cpp
  kbookmarkmenu.cpp
  kbookmarkowner.cpp
  konqbookmarkmenu.cpp
  kbookmarkimporter_opera.cpp
  kbookmarkimporter_ie.cpp
  kbookmarkimporter_ns.cpp
  kbookmarkdombuilder.cpp
  kbookmarkdialog.cpp
)

add_library(KF5Bookmarks ${kbookmarks_SRCS})
generate_export_header(KF5Bookmarks BASE_NAME KBookmarks)
add_library(KF5::Bookmarks ALIAS KF5Bookmarks)

target_include_directories(KF5Bookmarks INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KBookmarks>")

target_link_libraries(KF5Bookmarks PUBLIC Qt5::Widgets Qt5::Xml)
target_link_libraries(KF5Bookmarks PRIVATE
    Qt5::DBus # dbus usage in kbookmarkmanager.cpp
    KF5::ConfigWidgets # for KStandardAction
    KF5::XmlGui # for KActionCollection
    KF5::IconThemes # for SmallIcon
)

set_target_properties(KF5Bookmarks PROPERTIES VERSION   ${KBOOKMARKS_VERSION_STRING}
                                              SOVERSION ${KBOOKMARKS_SOVERSION}
                                              EXPORT_NAME Bookmarks
)

ecm_generate_headers(
  KBookmark
  KBookmarkAction
  KBookmarkActionInterface
  KBookmarkActionMenu
  KBookmarkContextMenu
  KBookmarkManager
  KBookmarkMenu
  KBookmarkOwner
  KBookmarkDomBuilder
  KBookmarkDialog
  KonqBookmarkMenu

  MODULE_NAME KBookmarks
  REQUIRED_HEADERS KBookmarks_HEADERS
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KBookmarks DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KF5Bookmarks EXPORT KF5BookmarksTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  # These classes have weirdly named classes, no forwarding headers.
  kbookmarkexporter.h
  kbookmarkimporter.h
  kbookmarkimporter_opera.h
  kbookmarkimporter_ie.h
  kbookmarkimporter_ns.h
  ${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_export.h
  ${KBookmarks_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KBookmarks COMPONENT Devel
)
include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KBookmarks LIB_NAME KF5Bookmarks DEPS "widgets xml" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
