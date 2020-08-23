#ifndef PROCEDURE_H
#define PROCEDURE_H


#include <QObject>
#include "process.h"


//pozdeji vice senderu...
class Procedure : public QObject
{
    Q_OBJECT

public:
    explicit Procedure(QString procedureNameInit, QObject *parent = 0);
    ~Procedure();
    QString procedureName;
    //refactoring needed
    QList<Process*> senders;
    bool addSender(Process* senderProcess);
    void removeSender(Process* senderProcess);
};

#endif // PROCEDURE_H
