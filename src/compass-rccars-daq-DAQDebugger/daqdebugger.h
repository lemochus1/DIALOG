#ifndef DAQDEBUGGER_H
#define DAQDEBUGGER_H

#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <errno.h>
#include <cxxabi.h>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "qthreaddaqdebugger.h"

class QThreadDAQDebugger;

class DAQDebugger : public QObject
{
    Q_OBJECT

    static QString process;
    static QString processName;
    static QMutex mutex;
    static QMap<Qt::HANDLE, QThreadDAQDebugger*> threads;
    static Qt::HANDLE crashedThread;
    static FILE* pFile;
    static quint32 threadCounter;
    static sigset_t sigActionThreadControlSignalMask;
    static struct sigaction sigActionThreadControlSignal;
    static struct sigaction sigActionThreadControlSuspend;
    static QString readSystemCommand(QString command);
    static void printStackTrace(FILE *out, unsigned int max_frames);
    static void getThreadStackTrace(int signum);
    static void abortHandler2(int signum);
    static void suspendHandler(int signum);

public:
    static void abortHandler(int signum);
    static void init(QString processInit, QString processNameInit = "");
    static void addThreadSlot(QThreadDAQDebugger* thread);
    static void removeThreadSlot(QThreadDAQDebugger* thread);
};

#endif // DAQDEBUGGER_H
