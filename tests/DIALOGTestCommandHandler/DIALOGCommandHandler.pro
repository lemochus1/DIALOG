QT -= gui

QT       += core network

QT       -= gui

TARGET = _rccars_DIALOGCommunicationControlServer
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
        main.cpp \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/release/ -lDAQDebugger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/debug/ -lDAQDebugger
else:unix: LIBS += -L$$PWD/../../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug/ -lDAQDebugger

INCLUDEPATH += $$PWD/../../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug
DEPENDPATH += $$PWD/../../build/build-DAQDebugger-Desktop_Qt_5_9_7_GCC_64bit-Debug

INCLUDEPATH += $$PWD/../../compass-rccars-daq-DAQDebugger
DEPENDPATH += $$PWD/../../compass-rccars-daq-DAQDebugger


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/release/ -lDIALOGCommunication
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/debug/ -lDIALOGCommunication
else:unix: LIBS += -L$$PWD/../../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug/ -lDIALOGCommunication

INCLUDEPATH += $$PWD/../../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug
DEPENDPATH += $$PWD/../../build/build-DIALOGCommunication-Desktop_Qt_5_9_7_GCC_64bit-Debug

INCLUDEPATH += $$PWD/../../compass-rccars-daq-DIALOGCommunication
DEPENDPATH += $$PWD/../../compass-rccars-daq-DIALOGCommunication

HEADERS += \

DISTFILES += \
    main.py \
    test.py

