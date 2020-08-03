#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QObject>
#include <QXmlStreamReader>
#include "server.h"

class DIALOGInfo : public QObject
{
    Q_OBJECT
public:
    explicit DIALOGInfo(QObject *parent = 0);
    ~DIALOGInfo();
    void readDIALOGInfo(QString info);
    void readDIALOGInfoProcessElement();
    void readDIALOGInfoServiceOrCommandElement(Process* process);
    void readDIALOGInfoReceiverElement(Service* service);
    void skipUnknownElement();
    void addProcess(Process* process);
    void addService(Service* service);
    void addCommand(Command* command);
    void clear();
    QXmlStreamReader XMLReader;
    QMap<QString, Process*> processes;
    QMap<QString, Service*> services;
    QMap<QString, Command*> commands;
};

#endif // DIALOGINFO_H
