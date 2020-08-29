#ifndef TESTCOMMANDSENDER_H
#define TESTCOMMANDSENDER_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTCommandSender : public QObject
{
    Q_OBJECT

public:
    TESTCommandSender(QString nameInit, int pauseInit = 1,
                      int repeatInit = 1, int messageSizeInit = 0);

    void setTargetAddress(QString processNameInit);
    void setTargetAddress(QString addressInit, int portInit);

    void start();

public slots:
    void sendCommand();

private:
    QByteArray generateMessage();
    QString commandName;

    QString processName;

    int port;
    QString address;
    QString targetProcessName;

    bool toName;
    bool toAddress;

    int repeat;
    int pause;
    int size;

    int sendCounter;
    QTimer* timer;
};

#endif // TESTCOMMANDSENDER_H
