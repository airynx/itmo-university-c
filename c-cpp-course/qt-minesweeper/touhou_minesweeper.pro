8QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    src/dialogsetter.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/minesweepercell.cpp \
    src/squarebutton.cpp \
    src/textalignableqlabel.cpp

HEADERS += \
    include/dialogsetter.h \
    include/mainwindow.h \
    include/minesweepercell.h \
    include/squarebutton.h \
    include/textalignableqlabel.h

CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += resources/resources.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
