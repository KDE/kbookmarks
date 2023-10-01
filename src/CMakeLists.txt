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
    kbookmarkactionmenu.cpp
    kbookmarkactionmenu.h
    kbookmarkcontextmenu.cpp
    kbookmarkcontextmenu.h
    kbookmark.cpp
    kbookmarkdialog.cpp
    kbookmarkdialog.h
    kbookmarkdialog_p.h
    kbookmark.h
    kbookmarkmanager.cpp
    kbookmarkmanager.h
    kbookmarkmenu.cpp
    kbookmarkmenu.h
    kbookmarkmenu_p.h
    kbookmarkowner.cpp
    kbookmarkowner.h
    keditbookmarks_p.h
    keditbookmarks.cpp
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

target_link_libraries(KF6Bookmarks PUBLIC Qt6::Widgets Qt6::Xml KF6::WidgetsAddons)
target_link_libraries(KF6Bookmarks PRIVATE
    KF6::CoreAddons # for KStringHandler
    KF6::ConfigCore # for KConfigGroup
    KF6::ConfigWidgets # for KStandardAction
)

ecm_generate_headers(KBookmarks_HEADERS
  HEADER_NAMES
  KBookmark
  KBookmarkAction
  KBookmarkActionInterface
  KBookmarkActionMenu
  KBookmarkContextMenu
  KBookmarkManager
  KBookmarkMenu
  KBookmarkOwner
  KBookmarkDialog

  REQUIRED_HEADERS KBookmarks_HEADERS
)

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

if(BUILD_QCH)
    ecm_add_qch(
        KF6Bookmarks_QCH
        NAME KBookmarks
        BASE_NAME KF6Bookmarks
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KBookmarks_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Widgets_QCH
            Qt6Xml_QCH
            KF6WidgetsAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KBOOKMARKS_EXPORT
            KBOOKMARKS_DEPRECATED
            KBOOKMARKS_DEPRECATED_EXPORT
            "KBOOKMARKS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

