#add_subdirectory(doc)

include_directories (
    ${KDE1_INCLUDE_DIR}
)

set(kmidi_SRCS
    kmidi.cpp  playlist.cpp bwlednum.cpp configdlg.cpp log.cpp
    timidity.c common.c readmidi.c playmidi.c resample.c mix.c instrum.c
    tables.c controls.c output.c filter.c wave_a.c raw_a.c dumb_c.c
    sndfont.c readsbk.c linux_a.c ctl.c dec_a.c sun_a.c hpux_a.c
    b_out.c fffload.c
)

set(kmidi_MOC
    bwlednum.moc
    configdlg.moc
    kmidi.moc
    log.moc
    playlist.moc
)

QT1_WRAP_MOC(MOC_FILES ${kmidi_MOC})

add_executable(kmidi ${kmidi_SRCS} ${MOC_FILES} ${QwSpriteField_SOURCES})
target_compile_definitions(kmidi PRIVATE KMIDI)
target_compile_definitions(kmidi PRIVATE -DDEFAULT_PATH=\"${KDE1_DATADIR}/kmidi/config/\")
target_link_libraries(kmidi
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    ${KDE1_KFILE}
)

install(TARGETS kmidi RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES KMidi.kdelnk DESTINATION ${KDE1_APPSDIR}/Multimedia)

install_icon(kmidi.xpm)

install(FILES
    bitmaps/cdup.bmp
    bitmaps/eject.xbm
    bitmaps/ff.xbm
    bitmaps/file.bmp
    bitmaps/folder.bmp
    bitmaps/folder.xpm
    bitmaps/info.xbm
    bitmaps/lock.xbm
    bitmaps/logo.xbm
    bitmaps/midilogo.xbm
    bitmaps/nexttrk.xbm
    bitmaps/options.xbm
    bitmaps/playpaus.xbm
    bitmaps/poweroff.xbm
    bitmaps/prevtrk.xbm
    bitmaps/repeat.xbm
    bitmaps/rew.xbm
    bitmaps/shuffle.xbm
    bitmaps/speeddown.xbm
    bitmaps/speedup.xbm
    bitmaps/stop.xbm
    bitmaps/time.xbm
    kmidilogo.xpm
    DESTINATION "${KDE1_DATADIR}/kmidi/pics/"
)

install(FILES
    config/gravis.cfg
    config/gsdrum.cfg
    config/midia.cfg
    config/mt32.cfg
    config/sndfont.cfg
    config/timidity.cfg
    config/wowpats.cfg
    config/chaos12-voices
    config/chaos8-voices
    config/megadrum
    config/megainst
    config/pila-voices
    config/sound-canvas-drums
    DESTINATION "${KDE1_DATADIR}/kmidi/config/"
)

install(FILES
    patch/acpiano.pat
    patch/nylongt2.pat
    DESTINATION "${KDE1_DATADIR}/kmidi/config/patch/"
    )

# TODO: german doc
install(FILES
    docs/kmidi-1.html
    docs/kmidi-2.html
    docs/kmidi-3.html
    docs/kmidi-4.html
    docs/kmidi-5.html
    docs/kmidi.html
    docs/kmidi.nif
    docs/kmidi.sgml
    docs/logotp3.gif
    DESTINATION ${KDE1_HTMLDIR}/en/kmidi)

