#ifndef GUIINFO_H
#define GUIINFO_H

#include <QObject>
#include <QXmlStreamReader>
#include "server.h"

class GUIInfo : public QObject
{
    Q_OBJECT
public:
    explicit GUIInfo(QObject *parent = 0);
    ~GUIInfo();
    void readGUIInfo(QString info);
    void readGUIInfoProcessElement();
    void readGUIInfoServiceOrCommandElement(Process* process);
    void readGUIInfoReceiverElement(Service* service);
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

#endif // GUIINFO_H
