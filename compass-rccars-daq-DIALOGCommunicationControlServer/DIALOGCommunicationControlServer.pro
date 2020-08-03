#-------------------------------------------------
#
# Project created by QtCreator 2015-10-13T10:57:51
#
#-------------------------------------------------


QT       += core network

QT       -= gui

TARGET = _rccars_DIALOGCommunicationControlServer
CONFIG   += console
CONFIG   -= app_bundle
#INCLUDEPATH += ../compass-rccars-daq-DIALOGCommunication
#DEPENDPATH += ../compass-rccars-daq-DIALOGCommunication
#LIBS += -L../compass-rccars-daq-DIALOGCommunication -lDIALOGCommunication



## DAQDebugger Start
#INCLUDEPATH += ../compass-rccars-daq-DAQDebugger/
#DEPENDPATH += ../compass-rccars-daq-DAQDebugger/
#LIBS += -L../compass-rccars-daq-DAQDebugger -lDAQDebugger

#QMAKE_CXXFLAGS += -rdynamic -g
#QMAKE_LFLAGS += -rdynamic -g

#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger.h
#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger_macro.h
#QMAKE_CXXFLAGS += -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtCore
#QMAKE_CXXFLAGS += -fPIC
# DAQDebugger End

TEMPLATE = app

SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/release/ -lDAQDebugger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/debug/ -lDAQDebugger
else:unix: LIBS += -L$$PWD/../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/ -lDAQDebugger

INCLUDEPATH += $$PWD/../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug
DEPENDPATH += $$PWD/../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug

INCLUDEPATH += $$PWD/../compass-rccars-daq-DAQDebugger
DEPENDPATH += $$PWD/../compass-rccars-daq-DAQDebugger


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/release/ -lDIALOGCommunication
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/debug/ -lDIALOGCommunication
else:unix: LIBS += -L$$PWD/../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/ -lDIALOGCommunication

INCLUDEPATH += $$PWD/../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug
DEPENDPATH += $$PWD/../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug

INCLUDEPATH += $$PWD/../compass-rccars-daq-DIALOGCommunication
DEPENDPATH += $$PWD/../compass-rccars-daq-DIALOGCommunication
