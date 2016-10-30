QT *= core\
    serialport

QT -= gui

CONFIG += c++11

TARGET = ninebituart

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

MY_BUILD_DIR = $$PWD/bin
DESTDIR = $$MY_BUILD_DIR

INCLUDEPATH *= \
    $$PWD\
    $$PWD/src

DEPENDPATH *=  \
    $$PWD/src

HEADERS *= \
    $$PWD/src/uartcommunicator.h

SOURCES *= \
    $$PWD/src/main.cpp \
    $$PWD/src/uartcommunicator.cpp



