ecm_create_qm_loader(kbookmarks_QM_LOADER kbookmarks5_qt)

set(kbookmarks_SRCS
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
    kbookmarkexporter.h
    kbookmark.h
    kbookmarkimporter.cpp
    kbookmarkimporter.h
    kbookmarkimporter_ie.cpp
    kbookmarkimporter_ie.h
    kbookmarkimporter_ns.cpp
    kbookmarkimporter_ns.h
    kbookmarkimporter_opera.cpp
    kbookmarkimporter_opera.h
    kbookmarkimporter_opera_p.h
    kbookmarkmanager.cpp
    kbookmarkmanager.h
    kbookmarkmenu.cpp
    kbookmarkmenu.h
    kbookmarkmenu_p.h
    kbookmarkowner.cpp
    kbookmarkowner.h
    konqbookmarkmenu.cpp
    konqbookmarkmenu.h
    konqbookmarkmenu_p.h
    kbookmarkdombuilder.cpp
    kbookmarkdombuilder.h
  ${kbookmarks_QM_LOADER}
)
if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
  list(APPEND kbookmarks_SRCS kbookmarkmanageradaptor.cpp kbookmarkmanageradaptor_p.h)
endif()

ecm_qt_declare_logging_category(kbookmarks_SRCS
    HEADER kbookmarks_debug.h
    IDENTIFIER KBOOKMARKS_LOG
    CATEGORY_NAME kf.bookmarks
    OLD_CATEGORY_NAMES kf5.kbookmarks
    DESCRIPTION "KBookmarks"
    EXPORT KBOOKMARKS
)

add_library(KF5Bookmarks ${kbookmarks_SRCS})
add_library(KF5::Bookmarks ALIAS KF5Bookmarks)
ecm_generate_export_header(KF5Bookmarks
    BASE_NAME KBookmarks
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.0 5.65 5.69
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Bookmarks INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarks>")

target_link_libraries(KF5Bookmarks PUBLIC Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Xml KF5::WidgetsAddons)
target_link_libraries(KF5Bookmarks PRIVATE
    KF5::CoreAddons # for KStringHandler
    KF5::Codecs # for KCharsets
    KF5::ConfigCore # for KConfigGroup
    KF5::ConfigWidgets # for KStandardAction
)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.69.0)
  target_link_libraries(KF5Bookmarks PRIVATE
    KF5::XmlGui # for KActionCollection
  )
endif()

if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
  target_link_libraries(KF5Bookmarks PRIVATE Qt${QT_MAJOR_VERSION}::DBus) # dbus usage in kbookmarkmanager.cpp
else()
  target_compile_definitions(KF5Bookmarks PUBLIC -DKBOOKMARKS_NO_DBUS)
endif()

set_target_properties(KF5Bookmarks PROPERTIES VERSION   ${KBOOKMARKS_VERSION}
                                              SOVERSION ${KBOOKMARKS_SOVERSION}
                                              EXPORT_NAME Bookmarks
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
  KBookmarkDomBuilder
  KBookmarkDialog
  KonqBookmarkMenu

  REQUIRED_HEADERS KBookmarks_HEADERS
)

# These classes have weirdly named classes, no forwarding headers.
set(KBookmarks_HEADERS
  ${KBookmarks_HEADERS}
  kbookmarkexporter.h
  kbookmarkimporter.h
  kbookmarkimporter_opera.h
  kbookmarkimporter_ie.h
  kbookmarkimporter_ns.h
)
install(TARGETS KF5Bookmarks EXPORT KF5BookmarksTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

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
        KF5Bookmarks_QCH
        NAME KBookmarks
        BASE_NAME KF5Bookmarks
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KBookmarks_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Widgets_QCH
            Qt5Xml_QCH
            KF5WidgetsAddons_QCH
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

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KBookmarks LIB_NAME KF5Bookmarks DEPS "widgets xml" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KBookmarks)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
