#include "qthreaddaqdebugger.h"

QThreadDAQDebugger::QThreadDAQDebugger(QObject *parent)
    : QThread(parent)
{
    qThreadDAQDebuggerHelper = new QThreadDAQDebuggerHelper();
    qThreadDAQDebuggerHelper->moveToThread(this);

    QObject::connect(this, &QThreadDAQDebugger::started, qThreadDAQDebuggerHelper, &QThreadDAQDebuggerHelper::addThreadSlot);
    QObject::connect(this, &QThreadDAQDebugger::finished, this, &QThreadDAQDebugger::removeThreadSlot);
    QObject::connect(this, &QThreadDAQDebugger::removeThreadSignal, DAQDebugger::removeThreadSlot);
}

QThreadDAQDebugger::~QThreadDAQDebugger()
{
    delete qThreadDAQDebuggerHelper;
}

QThreadDAQDebugger* QThreadDAQDebugger::currentThread()
{
    return static_cast<QThreadDAQDebugger*>(QThread::currentThread());
}

void QThreadDAQDebugger::removeThreadSlot()
{
    Q_EMIT removeThreadSignal(this);
}
