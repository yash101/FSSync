TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CXXFLAGS += -DDEBUG

SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-fssynclib-Desktop_Qt_5_8_0_GCC_64bit-Debug/release/ -lfssynclib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-fssynclib-Desktop_Qt_5_8_0_GCC_64bit-Debug/debug/ -lfssynclib
else:unix: LIBS += -L$$PWD/../build-fssynclib-Desktop_Qt_5_8_0_GCC_64bit-Debug/ -lfssynclib

INCLUDEPATH += $$PWD/../fssynclib
DEPENDPATH += $$PWD/../fssynclib
