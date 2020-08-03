#ifndef SENDERPROCESSORTHREAD_H
#define SENDERPROCESSORTHREAD_H

#include <QObject>
#include <iostream>
#include <server.h>

class SenderProcessorThread : public VirtualThread
{
    Q_OBJECT
public:
    ~SenderProcessorThread();
    void run();

public slots:
    void requestServiceSlot(QString serviceName);
    void unSubscribeServiceSlot(QString serviceName);
    void sendCommandMessageSlot(QString commandName, QString message);
    void registerCommandSlot(QString commandName);
    void unRegisterCommandSlot(QString commandName);
};

#endif // SENDERPROCESSORTHREAD_H
