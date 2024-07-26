QT = core network

CONFIG += c++11 cmdline

SOURCES += \
        am5broadcast.cpp \
        hostinfo.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    am5broadcast.h \
    hostinfo.h
