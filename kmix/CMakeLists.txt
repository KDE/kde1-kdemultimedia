#add_subdirectory(doc)

include_directories (
    ${KDE1_INCLUDE_DIR}
)

set(kmix_SRCS
    channel.cpp
    docking.cpp
    kmix.cpp
    mixer.cpp
    prefs.cpp
    sets.cpp
)

set(kmix_MOC
    docking.h
    kmix.h
    mixer.h
    mixer.h
    prefs.h
)

QT1_WRAP_MOC(MOC_FILES ${kmix_MOC})

add_executable(kmix ${kmix_SRCS} ${MOC_FILES} ${QwSpriteField_SOURCES})
target_compile_definitions(kmix PRIVATE ALSA)
target_link_libraries(kmix
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    ${KDE1_KFILE}
)

install(TARGETS kmix RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES KMix.kdelnk DESTINATION ${KDE1_APPSDIR}/Multimedia)

install_icon(pics/kmix.xpm)

install(FILES
    mix_audio.xpm mix_bass.xpm mix_cd.xpm mix_ext.xpm
    mix_microphone.xpm mix_midi.xpm mix_recmon.xpm mix_treble.xpm
    mix_unknown.xpm mix_volume.xpm kmixdocked.xpm
    DESTINATION "${KDE1_DATADIR}/kmix/pics/")

install(FILES
    index-1.html index-2.html index-3.html index-4.html
    index-5.html index-6.html index.html logotp3.gif
    DESTINATION ${KDE1_HTMLDIR}/en/kmix)
