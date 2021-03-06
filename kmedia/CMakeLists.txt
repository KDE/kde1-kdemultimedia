#add_subdirectory(doc)

include_directories (
    ${KDE1_INCLUDE_DIR}
)

set(kmedia_SRCS
    kmediaui.cpp
    kmediawin.cpp
    mediaactions.cpp
    prefs.cpp
    uiactions.cpp
)

set(kmedia_MOC
    kmediaui.moc
    kmediawin.moc
    prefs.moc
    version.moc
    )

QT1_WRAP_MOC(MOC_FILES ${kmedia_MOC})

add_executable(kmedia ${kmedia_SRCS} ${MOC_FILES} ${QwSpriteField_SOURCES})
target_link_libraries(kmedia
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    ${KDE1_KFILE}
)

install(TARGETS kmedia RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES KMedia.kdelnk DESTINATION ${KDE1_APPSDIR}/Multimedia)

add_subdirectory(pics)

install(FILES
    docs/index-1.html
    docs/index-2.html
    docs/index-3.html
    docs/index-4.html
    docs/index-5.html
    docs/index.html
    docs/logotp3.gif
DESTINATION ${KDE1_HTMLDIR}/en/kmedia)
