#-------------------------------------------------
#
# Project created by QtCreator 2015-10-05T09:57:28
#
#-------------------------------------------------

QT += network

TARGET = DIALOGCommunication
TEMPLATE = lib

RCCARS_DAQ_HOME = $$(RCCARS_HOME)/compass-rccars-daq

# DAQDebugger Start
INCLUDEPATH += $${RCCARS_DAQ_HOME}/compass-rccars-daq-DAQDebugger/
DEPENDPATH += .$${RCCARS_DAQ_HOME}/compass-rccars-daq-DAQDebugger/
LIBS += -L$${RCCARS_DAQ_HOME}/compass-rccars-daq-DAQDebugger -lDAQDebugger

#QMAKE_CXXFLAGS += -rdynamic -g
#QMAKE_LFLAGS += -rdynamic -g

#QMAKE_CXXFLAGS += -include $${RCCARS_DAQ_HOME}/compass-rccars-daq-DAQDebugger/qthreaddaqdebugger.h
#QMAKE_CXXFLAGS += -include $${RCCARS_DAQ_HOME}/compass-rccars-daq-DAQDebugger/qthreaddaqdebugger_macro.h
#QMAKE_CXXFLAGS += -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtCore
#QMAKE_CXXFLAGS += -fPIC
# DAQDebugger End


HEADERS       = \
    dialogapi.h \
    procedure.h \
                sender.h \
                receiver.h \
    socket.h \
    server.h \
    service.h \
    process.h \
    define.h \
    message.h \
    command.h \
    virtualthread.h \
    tcpserver.h \
    messagecontainer.h \
    threadsafeqlist.h
SOURCES       = \
    dialogapi.cpp \
    procedure.cpp \
                sender.cpp \
                receiver.cpp \
    socket.cpp \
    server.cpp \
    service.cpp \
    process.cpp \
    message.cpp \
    command.cpp \
    virtualthread.cpp \
    tcpserver.cpp \
    messagecontainer.cpp \
    threadsafeqlist.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/DAQDebugger/release/ -lDAQDebugger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/DAQDebugger/debug/ -lDAQDebugger
else:unix: LIBS += -L$$PWD/../../build/DAQDebugger/ -lDAQDebugger

INCLUDEPATH += $$PWD/../../build/DAQDebugger
DEPENDPATH += $$PWD/../../build/DAQDebugger
