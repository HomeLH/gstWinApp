#-------------------------------------------------
#
# Project created by QtCreator 2019-08-07T16:15:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += core quick opengl gamepad xml svg qml multimedia multimediawidgets


TARGET = gstf
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

#CONFIG(debug, debug|release) {
#    DESTDIR  = $${OUT_PWD}/debug
#    CONFIG  += DebugBuild
#    CONFIG      += VideoEnabled
#} else {
#    DESTDIR  = $${OUT_PWD}/release
#    CONFIG  += ReleaseBuild
#    win32: QMAKE_CXXFLAGS += /bigobj
#    CONFIG      += VideoEnabled
#}

GST_ROOT = D:\gstreamer\1.0\x86_64
#DEFINES += QT_DEPRECATED_WARNINGS
LIBS        += -L$$GST_ROOT/lib -lgstreamer-1.0 -lgstvideo-1.0 -lgstbase-1.0
LIBS        += -lglib-2.0 -lintl -lgobject-2.0



INCLUDEPATH += \
    $$GST_ROOT/include/gstreamer-1.0 \
    $$GST_ROOT/include/glib-2.0 \
    $$GST_ROOT/lib/gstreamer-1.0/include \
    $$GST_ROOT/lib/glib-2.0/include

GTK_ROOT = C:/msys32/mingw32
INCLUDEPATH += \
    $$GTK_ROOT/include/gtk-3.0\

DESTDIR_WIN = $$replace(DESTDIR, "/", "\\")
GST_ROOT_WIN = $$replace(GST_ROOT, "/", "\\")

## Copy main GStreamer runtime files
QMAKE_POST_LINK += $$escape_expand(\\n) xcopy \"$$GST_ROOT_WIN\\bin\*.dll\" \"$$DESTDIR_WIN\" /S/Y $$escape_expand(\\n)
QMAKE_POST_LINK += xcopy \"$$GST_ROOT_WIN\\bin\*.\" \"$$DESTDIR_WIN\" /S/Y $$escape_expand(\\n)

## Copy GStreamer plugins
QMAKE_POST_LINK += $$escape_expand(\\n) xcopy \"$$GST_ROOT_WIN\\lib\\gstreamer-1.0\\*.dll\" \"$$DESTDIR_WIN\\gstreamer-plugins\\\" /Y $$escape_expand(\\n)
#QMAKE_POST_LINK += $$escape_expand(\\n) xcopy \"$$GST_ROOT_WIN\\lib\\gstreamer-1.0\\validate\\*.dll\" \"$$DESTDIR_WIN\\gstreamer-plugins\\validate\\\" /Y $$escape_expand(\\n)


VideoEnabled {

    message("Including support for video streaming")

    DEFINES += \
        QGC_GST_STREAMING \
        GST_PLUGIN_BUILD_STATIC \
        QTGLVIDEOSINK_NAME=qt5glvideosink \
        QGC_VIDEOSINK_PLUGIN=qt5videosink

    INCLUDEPATH += \
        $$PWD/gstqtvideosink \
        $$PWD/gstqtvideosink/delegates \
        $$PWD/gstqtvideosink/painters \
        $$PWD/gstqtvideosink/utils \


    HEADERS += \
        $$PWD/gstqtvideosink/delegates/basedelegate.h \
        $$PWD/gstqtvideosink/delegates/qtquick2videosinkdelegate.h \
        $$PWD/gstqtvideosink/delegates/qtvideosinkdelegate.h \
        $$PWD/gstqtvideosink/delegates/qwidgetvideosinkdelegate.h \
        $$PWD/gstqtvideosink/gstqtglvideosink.h \
        $$PWD/gstqtvideosink/gstqtglvideosinkbase.h \
        $$PWD/gstqtvideosink/gstqtquick2videosink.h \
        $$PWD/gstqtvideosink/gstqtvideosink.h \
        $$PWD/gstqtvideosink/gstqtvideosinkbase.h \
        $$PWD/gstqtvideosink/gstqtvideosinkmarshal.h \
        $$PWD/gstqtvideosink/gstqtvideosinkplugin.h \
        $$PWD/gstqtvideosink/gstqwidgetvideosink.h \
        $$PWD/gstqtvideosink/painters/abstractsurfacepainter.h \
        $$PWD/gstqtvideosink/painters/genericsurfacepainter.h \
        $$PWD/gstqtvideosink/painters/openglsurfacepainter.h \
        $$PWD/gstqtvideosink/painters/videomaterial.h \
        $$PWD/gstqtvideosink/painters/videonode.h \
        $$PWD/gstqtvideosink/utils/bufferformat.h \
        $$PWD/gstqtvideosink/utils/utils.h \
        $$PWD/gstqtvideosink/utils/glutils.h \

    SOURCES += \
        $$PWD/gstqtvideosink/delegates/basedelegate.cpp \
        $$PWD/gstqtvideosink/delegates/qtquick2videosinkdelegate.cpp \
        $$PWD/gstqtvideosink/delegates/qtvideosinkdelegate.cpp \
        $$PWD/gstqtvideosink/delegates/qwidgetvideosinkdelegate.cpp \
        $$PWD/gstqtvideosink/gstqtglvideosink.cpp \
        $$PWD/gstqtvideosink/gstqtglvideosinkbase.cpp \
        $$PWD/gstqtvideosink/gstqtvideosinkmarshal.c \
        $$PWD/gstqtvideosink/gstqtquick2videosink.cpp \
        $$PWD/gstqtvideosink/gstqtvideosink.cpp \
        $$PWD/gstqtvideosink/gstqtvideosinkbase.cpp \
        $$PWD/gstqtvideosink/gstqtvideosinkplugin.cpp \
        $$PWD/gstqtvideosink/gstqwidgetvideosink.cpp \
        $$PWD/gstqtvideosink/painters/genericsurfacepainter.cpp \
        $$PWD/gstqtvideosink/painters/openglsurfacepainter.cpp \
        $$PWD/gstqtvideosink/painters/videomaterial.cpp \
        $$PWD/gstqtvideosink/painters/videonode.cpp \
        $$PWD/gstqtvideosink/utils/bufferformat.cpp \
        $$PWD/gstqtvideosink/utils/utils.cpp \

} else {
    linux|macx|ios|win32|android-g++ {
        message("Skipping support for video streaming (GStreamer libraries not installed)")
        macx {
            message("  You can download it from http://gstreamer.freedesktop.org/data/pkg/osx/")
            message("  Select the devel package and install it (gstreamer-1.0-devel-1.x.x-x86_64.pkg)")
            message("  It will be installed in /Libraries/Frameworks")
        }
        linux {
            message("  You can install it using apt-get")
            message("  sudo apt-get install gstreamer1.0*")
        }
        win32 {
            message("  You can download it from http://gstreamer.freedesktop.org/data/pkg/windows/")
            message("  Select the devel AND runtime packages and install them (x64 and x32)")
            message("  It will be installed in C:/gstreamer. You need to update you PATH to point to the bin directory.")
        }
        android-g++ {
            message("  You can download it from http://gstreamer.freedesktop.org/data/pkg/android/")
            message("  Uncompress the archive into the qgc root source directory (same directory where qgroundcontrol.pro is found.")
        }
    } else {
        message("Skipping support for video streaming (Unsupported platform)")
    }
}

SOURCES += \
        gstplayer.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        gstplayer.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
