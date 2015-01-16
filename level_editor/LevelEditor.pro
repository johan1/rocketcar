TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += /usr/include/jsoncpp
LIBS += -ljsoncpp

QMAKE_CXXFLAGS += -std=c++11

CONFIG += qt debug

# Input
HEADERS += common.h \
           config.h \
           leveleditor.h \
           mainwindow.h \
           newleveldialog.h \
		   editorgraphicsview.h

FORMS += mainwindow.ui newleveldialog.ui

SOURCES += config.cpp \
           leveleditor.cpp \
           main.cpp \
           mainwindow.cpp \
           newleveldialog.cpp \
           editorgraphicsview.cpp
