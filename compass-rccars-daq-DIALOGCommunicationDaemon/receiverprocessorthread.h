#ifndef RECEIVERPROCESSORTHREAD_H
#define RECEIVERPROCESSORTHREAD_H

#include <QObject>
#include <server.h>
#include "dialogInfo.h"

class ReceiverProcessorThread : public VirtualThread
{
  Q_OBJECT

public:
  ~ReceiverProcessorThread();
  void run();

public Q_SLOTS:
    void messageReceivedSlot(QString senderAddress, quint16 senderPort, QByteArray* header, QByteArray* message);

Q_SIGNALS:
    void updateDIALOGInfoSignal(DIALOGInfo* dialogInfo, DIALOGInfo* dialogInfoOld);
    void serviceOutputSignal(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message);
    void commandOutputSignal(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message);

private:
    DIALOGInfo* dialogInfo;
};

#endif // RECEIVERPROCESSORTHREAD_H
