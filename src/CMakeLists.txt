add_subdirectory(widgets)

add_library(KF6Bookmarks)
add_library(KF6::Bookmarks ALIAS KF6Bookmarks)

set_target_properties(KF6Bookmarks PROPERTIES
    VERSION     ${KBOOKMARKS_VERSION}
    SOVERSION   ${KBOOKMARKS_SOVERSION}
    EXPORT_NAME Bookmarks
)

ecm_create_qm_loader(KF6Bookmarks kbookmarks6_qt)

target_sources(KF6Bookmarks PRIVATE
    kbookmarkaction.cpp
    kbookmarkaction.h
    kbookmarkactioninterface.cpp
    kbookmarkactioninterface.h
    kbookmark.cpp
    kbookmark.h
    kbookmarkmanager.cpp
    kbookmarkmanager.h
    kbookmarkowner.cpp
    kbookmarkowner.h
    keditbookmarks_p.h
    keditbookmarks.cpp
    kbookmarksettings.cpp
)

ecm_qt_declare_logging_category(KF6Bookmarks
    HEADER kbookmarks_debug.h
    IDENTIFIER KBOOKMARKS_LOG
    CATEGORY_NAME kf.bookmarks
    OLD_CATEGORY_NAMES kf5.kbookmarks
    DESCRIPTION "KBookmarks"
    EXPORT KBOOKMARKS
)

ecm_generate_export_header(KF6Bookmarks
    BASE_NAME KBookmarks
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Bookmarks INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarks>")

target_link_libraries(KF6Bookmarks PUBLIC Qt6::Gui Qt6::Xml)
target_link_libraries(KF6Bookmarks PRIVATE
    KF6::CoreAddons # for KStringHandler
    KF6::ConfigCore # for KConfigGroup
)

ecm_generate_headers(KBookmarks_HEADERS
  HEADER_NAMES
  KBookmark
  KBookmarkAction
  KBookmarkActionInterface
  KBookmarkManager
  KBookmarkOwner

  REQUIRED_HEADERS KBookmarks_HEADERS
)

ecm_generate_qdoc(KF6Bookmarks kbookmarks.qdocconf)

install(TARGETS KF6Bookmarks EXPORT KF6BookmarksTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_export.h
  ${KBookmarks_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarks COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KBOOKMARKS
    FILE kbookmarks.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
