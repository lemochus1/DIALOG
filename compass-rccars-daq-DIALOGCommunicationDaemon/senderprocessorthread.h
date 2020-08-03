#ifndef SENDERPROCESSORTHREAD_H
#define SENDERPROCESSORTHREAD_H

#include <QObject>
#include <server.h>

class SenderProcessorThread : public VirtualThread
{
  Q_OBJECT

public:
  ~SenderProcessorThread();
  void run();

public Q_SLOTS:
    void requestServiceSlot(QString serviceName);
    void registerCommandSlot(QString commandName);
};

#endif // SENDERPROCESSORTHREAD_H
