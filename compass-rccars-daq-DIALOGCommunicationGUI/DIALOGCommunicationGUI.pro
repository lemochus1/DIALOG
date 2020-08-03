#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T13:01:09
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DIALOGCommunicationGUI

#INCLUDEPATH += ../compass-rccars-daq-DIALOGCommunication
#DEPENDPATH += ../compass-rccars-daq-DIALOGCommunication
#LIBS += -L../compass-rccars-daq-DIALOGCommunication -lDIALOGCommunication

TEMPLATE = app

# DAQDebugger Start
#INCLUDEPATH += ../compass-rccars-daq-DAQDebugger/
#DEPENDPATH += ../compass-rccars-daq-DAQDebugger/
#LIBS += -L../compass-rccars-daq-DAQDebugger -lDAQDebugger

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



#QMAKE_CXXFLAGS += -rdynamic -g
#QMAKE_LFLAGS += -rdynamic -g

#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger.h
#QMAKE_CXXFLAGS += -include ../compass-rccars-daq-DAQDebugger/qthreaddaqdebugger_macro.h
#QMAKE_CXXFLAGS += -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtCore
#QMAKE_CXXFLAGS += -fPIC
# DAQDebugger End

SOURCES += main.cpp\
        communicationgui.cpp \
    senderprocessorthread.cpp \
    receiverprocessorthread.cpp \
    guiinfo.cpp \
    servicedialog.cpp \
    showservicebutton.cpp \
    receiverdialog.cpp \
    showreceiverbutton.cpp \
    showcommandbutton.cpp \
    commanddialog.cpp

HEADERS  += communicationgui.h \
    senderprocessorthread.h \
    receiverprocessorthread.h \
    guiinfo.h \
    servicedialog.h \
    showservicebutton.h \
    receiverdialog.h \
    showreceiverbutton.h \
    showcommandbutton.h \
    commanddialog.h

FORMS    += communicationgui.ui \
    servicedialog.ui \
    receiverdialog.ui \
    commanddialog.ui
