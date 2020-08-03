#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include "process.h"

class Command : public QObject
{
    Q_OBJECT

public:
    explicit Command(QString commandNameInit, QObject *parent = 0);
    ~Command();
    QString commandName;
    QList<Process*> receivers;
    bool addReceiver(Process* receiverProcess);
    void removeReceiver(Process* receiverProcess);
};

#endif // COMMAND_H
