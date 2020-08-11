#ifndef TESTCOMMANDSENDER_H
#define TESTCOMMANDSENDER_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTCommandSender : public QObject
{
    Q_OBJECT

public:
    TESTCommandSender(QString nameInit, DIALOGProcess* processInit, int pauseInit = 1, int repeatInit = 1);

    void setTargetAddress(QString processNameInit);
    void setTargetAddress(QString addressInit, int portInit);

    void start();

public slots:
    void sendCommand();

signals:

private:
    QString commandName;

    DIALOGProcess* process;
    QString processName;

    int port;
    QString address;
    QString targetProcessName;

    bool toName;
    bool toAddress;

    int repeat;
    int pause;

    int sendCounter;
    QMutex mutex;
    QTimer* timer;
};

#endif // TESTCOMMANDSENDER_H
