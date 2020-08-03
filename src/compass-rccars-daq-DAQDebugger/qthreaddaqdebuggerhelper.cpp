#include "qthreaddaqdebuggerhelper.h"

QThreadDAQDebuggerHelper::QThreadDAQDebuggerHelper(QObject *parent)
    : QObject(parent)
{
    QObject::connect(this, &QThreadDAQDebuggerHelper::addThreadSignal, DAQDebugger::addThreadSlot);
}

void QThreadDAQDebuggerHelper::addThreadSlot()
{
    Q_EMIT addThreadSignal(QThreadDAQDebugger::currentThread());
}
