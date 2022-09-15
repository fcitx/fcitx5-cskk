# Icon is not a fcitx component, so install in CMAKE_INSTALL_DATADIR
foreach(size 24 32 48 256)
    install(DIRECTORY icon/${size}x${size} DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor"
        PATTERN .* EXCLUDE
        PATTERN *~ EXCLUDE)
endforeach(size 24 32 48 256)
