find_package(Qt6 REQUIRED COMPONENTS Multimedia)
lumen_add_app(lumen-mediaplayer URI Lumen.MediaPlayer LIBS Qt6::Multimedia DESKTOP lumen-mediaplayer.desktop)
