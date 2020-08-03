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
    Process* sender;
    void addSender(Process* senderProcess);
    void removeSender();
};

#endif // PROCEDURE_H
