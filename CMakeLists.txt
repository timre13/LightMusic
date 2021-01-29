CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(LightMusic VERSION 1.0)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED true)

LINK_LIBRARIES(
    avformat
    avcodec
    avdevice
    avutil
    swresample
    fltk
    X11
    fltk_images)

# Create an empty directory for the images
# Configure-time!
ADD_CUSTOM_TARGET(make-img-dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory img)

# Copy the icon to the destination
# Configure-time!
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/img/icon.png ${CMAKE_CURRENT_BINARY_DIR}/img/icon.png COPYONLY)

ADD_EXECUTABLE(lightmusic
    main.cpp
    Music.h
    Music.cpp
    Playlist.h
    Playlist.cpp
    MainWindow.h
    MainWindow.cpp
    AboutWindow.h
    AboutWindow.cpp
    sys-specific.h
    license.h 
    version.h
)
