#ifndef QTHREADDAQDEBUGGERHELPER_H
#define QTHREADDAQDEBUGGERHELPER_H

#include <QObject>
#include "qthreaddaqdebugger.h"
#include "daqdebugger.h"

class QThreadDAQDebugger;
class DAQDebugger;

class QThreadDAQDebuggerHelper : public QObject
{
    Q_OBJECT

public:
    explicit QThreadDAQDebuggerHelper(QObject *parent = Q_NULLPTR);

public Q_SLOTS:
    void addThreadSlot();

Q_SIGNALS:
    void addThreadSignal(QThreadDAQDebugger* thread);
};

#endif // QTHREADDAQDEBUGGERHELPER_H
