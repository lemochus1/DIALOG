#ifndef TESTPROCEDURECALLER_H
#define TESTPROCEDURECALLER_H

#include <QObject>
#include <QTimer>

#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTProcedureCallController : public QObject
{
    Q_OBJECT

public:
    TESTProcedureCallController(QString nameInit,
                                QString targetProcessInit,
                                int durationInit,
                                int repeatInit);

    void start();

public slots:
    void callProcedure();

    void controlServerUnavailable();
    void controlServerConnected();

    void responseReceived(const QByteArray &message);
    void callFailed(const QByteArray &message);
    void procedureUnavailable();
    void paramsInvalid();
    void timeoutPassed();

private:
    void waitForData(QSharedPointer<DIALOGProcedureCaller> caller);

    QString name;
    QString processName;
    QString targetProcess;
    int duration;
    int repeat;
    int callCounter;
    QTimer* timer;
};

#endif // TESTPROCEDURECALLER_H
