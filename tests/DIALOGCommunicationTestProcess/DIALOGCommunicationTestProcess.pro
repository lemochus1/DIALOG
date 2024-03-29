QT -= gui
QT += core network xml

TARGET = DIALOGTestProcess
CONFIG   += console
CONFIG   -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    apimessagelogger.cpp \
        main.cpp \
    testcommandhandler.cpp \
    testcommandsender.cpp \
    testprocedurecallcontroller.cpp \
    testprocedureprovider.cpp \
    testprocesscontroller.cpp \
    testservicepublisher.cpp \
    testservicesubscriber.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/build-DAQDebugger/release/ -lDAQDebugger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/build-DAQDebugger/debug/ -lDAQDebugger
else:unix: LIBS += -L$$PWD/../../build/DAQDebugger/ -lDAQDebugger

INCLUDEPATH += $$PWD/../../build/DAQDebugger
DEPENDPATH += $$PWD/../../build/DAQDebugger

INCLUDEPATH += $$PWD/../../src/compass-rccars-daq-DAQDebugger
DEPENDPATH += $$PWD/../../src/compass-rccars-daq-DAQDebugger


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/DIALOGCommunication/release/ -lDIALOGCommunication
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/DIALOGCommunication/debug/ -lDIALOGCommunication
else:unix: LIBS += -L$$PWD/../../build/DIALOGCommunication/ -lDIALOGCommunication

INCLUDEPATH += $$PWD/../../build/DIALOGCommunication
DEPENDPATH += $$PWD/../../build/DIALOGCommunication

INCLUDEPATH += $$PWD/../../src/compass-rccars-daq-DIALOGCommunication
DEPENDPATH += $$PWD/../../src/compass-rccars-daq-DIALOGCommunication

HEADERS += \
    apimessagelogger.h \
    testcommandhandler.h \
    testcommandsender.h \
    testprocedurecallcontroller.h \
    testprocedureprovider.h \
    testprocesscontroller.h \
    testservicepublisher.h \
    testservicesubscriber.h
