#ifndef RECEIVERPROCESSORTHREAD_H
#define RECEIVERPROCESSORTHREAD_H

#include <QObject>
#include <iostream>
#include <server.h>
#include "guiinfo.h"

class ReceiverProcessorThread : public VirtualThread
{
    Q_OBJECT

public:
    ~ReceiverProcessorThread();
    void run();

public slots:
    void messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray* header, QByteArray* message);
    void serverErrorSlot(QString error);

signals:
    void updateConnectedProcessesTableSignal(GUIInfo* guiInfo, GUIInfo* guiInfoOld);
    void serviceOutputSignal(QString serviceName, QByteArray message);
    void commandOutputSignal(QString commandName, QByteArray message);
    void communicationControlServerNotFoundSignal();

private:
    GUIInfo* guiInfo;
};

#endif // RECEIVERPROCESSORTHREAD_H
