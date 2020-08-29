#ifndef PROCEDURE_H
#define PROCEDURE_H

#include <QObject>
#include "process.h"

class Procedure : public QObject
{
    Q_OBJECT

public:
    explicit Procedure(QString procedureNameInit, QObject *parent = 0);
    ~Procedure();
    QString procedureName;
    QList<Process*> senders;
    bool addSender(Process* senderProcess);
    void removeSender(Process* senderProcess);
    /// Uniformly distributes calls.
    Process* getNextSender();

private:
    int lastSenderIndex;
};

#endif // PROCEDURE_H
