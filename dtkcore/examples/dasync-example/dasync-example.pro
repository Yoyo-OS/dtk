######################################################################
# Automatically generated by qmake (3.1) Thu Aug 19 09:48:31 2021
######################################################################

TEMPLATE = app
TARGET = thread_util
INCLUDEPATH += .
QT+= core widgets testlib

CONFIG += c++11
# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
    LIBS += -lgtest -lgmock
    QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -fsanitize=address -fsanitize-recover=address -O2
    QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -fsanitize=address -fsanitize-recover=address -O2
    QMAKE_CXX += -g -fprofile-arcs -ftest-coverage -fsanitize=address -fsanitize-recover=address -O2
}

LIBS += -pthread
QMAKE_CXXFLAGS += -pthread

#QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden
#DEFINES += LIBDTKCORE_LIBRARY

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../../src
INCLUDEPATH += $$PWD/../../src/base
INCLUDEPATH += $$PWD/../../src/util

# Input
HEADERS += \
    $${PWD}/../../src/dtkcore_global.h \
    $${PWD}/../../src/util/dasync.h \
    $${PWD}/../../src/util/dthreadutils.h

SOURCES += \
    $${PWD}/../../src/util/dthreadutils.cpp \
    main.cpp

