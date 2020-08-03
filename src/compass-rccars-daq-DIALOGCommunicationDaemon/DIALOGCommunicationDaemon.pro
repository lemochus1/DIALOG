#-------------------------------------------------
#
# Project created by QtCreator 2017-09-11T11:25:16
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = DIALOGCommunicationDaemon
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/DAQDebugger/release/ -lDAQDebugger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/DAQDebugger/debug/ -lDAQDebugger
else:unix: LIBS += -L$$PWD/../../build/DAQDebugger/ -lDAQDebugger

INCLUDEPATH += $$PWD/../../build/DAQDebugger
DEPENDPATH += $$PWD/../../build/DAQDebugger

INCLUDEPATH += $$PWD/../compass-rccars-daq-DAQDebugger
DEPENDPATH += $$PWD/../compass-rccars-daq-DAQDebugger


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/DIALOGCommunication/release/ -lDIALOGCommunication
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/DIALOGCommunication/debug/ -lDIALOGCommunication
else:unix: LIBS += -L$$PWD/../../build/DIALOGCommunication/ -lDIALOGCommunication

INCLUDEPATH += $$PWD/../../build/DIALOGCommunication
DEPENDPATH += $$PWD/../../build/DIALOGCommunication

INCLUDEPATH += $$PWD/../compass-rccars-daq-DIALOGCommunication
DEPENDPATH += $$PWD/../compass-rccars-daq-DIALOGCommunication




#QMAKE_CXXFLAGS += -rdynamic -g
#QMAKE_LFLAGS += -rdynamic -g

#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger.h
#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger_macro.h
#QMAKE_CXXFLAGS += -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtCore
#QMAKE_CXXFLAGS += -fPIC
# DAQDebugger End

SOURCES += main.cpp \
    senderprocessorthread.cpp \
    receiverprocessorthread.cpp \
    dialogInfo.cpp \
    daemonthread.cpp

HEADERS += \
    senderprocessorthread.h \
    receiverprocessorthread.h \
    dialogInfo.h \
    daemonthread.h \
    definedaemon.h
