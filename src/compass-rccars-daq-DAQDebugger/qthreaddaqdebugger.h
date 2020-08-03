#ifndef QTHREADDAQDEBUGGER_H
#define QTHREADDAQDEBUGGER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "daqdebugger.h"
#include "qthreaddaqdebuggerhelper.h"

class DAQDebugger;
class QThreadDAQDebuggerHelper;

class QThreadDAQDebugger : public QThread
{
    Q_OBJECT

    QThreadDAQDebuggerHelper* qThreadDAQDebuggerHelper;

public:
    explicit QThreadDAQDebugger(QObject *parent = Q_NULLPTR);
    ~QThreadDAQDebugger();
    static QThreadDAQDebugger* currentThread();

public Q_SLOTS:
    void removeThreadSlot();

Q_SIGNALS:
    void addThreadSignal(QThreadDAQDebugger* thread);
    void removeThreadSignal(QThreadDAQDebugger* thread);
};

#endif // QTHREADDAQDEBUGGER_H
