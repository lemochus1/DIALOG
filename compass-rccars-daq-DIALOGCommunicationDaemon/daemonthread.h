#ifndef DAEMONTHREAD_H
#define DAEMONTHREAD_H

#include <QObject>
#include <QEventLoop>
#include "dialogInfo.h"
#include "definedaemon.h"

class DaemonThread : public QThread
{
  Q_OBJECT

public:
  DaemonThread();
  ~DaemonThread();
  void stop();

public Q_SLOTS:
    void run();
    void updateDIALOGInfoSlot(DIALOGInfo* dialogInfoInit, DIALOGInfo* dialogInfoOld);
    void serviceOutputSlot(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message);
    void commandOutputSlot(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message);
    void postTimerSlot();
    void postRequestFinishedSlot(QNetworkReply* reply);
    void stopDaemonThreadSlot();

Q_SIGNALS:
    void requestServiceSignal(QString serviceName);
    void registerCommandSignal(QString commandName);
    void stopInitiated();

private:
    QThread* mainThread;
    QEventLoop* daemonThreadEventLoop;

    QString dialogControlServerAddress;
    quint16 dialogControlServerPort;
    QString dialogControlServerKey;
    DIALOGInfo* dialogInfo;

    QTimer* postTimer;

    // JSON
    QJsonArray* jsonArray;
};

#endif // DAEMONTHREAD_H
