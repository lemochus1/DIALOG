#ifndef TESTPROCEDUREHANDLER_H
#define TESTPROCEDUREHANDLER_H

#include <QObject>

#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTProcedureProvider : public DIALOGProcedureProvider
{
    Q_OBJECT

public:
    TESTProcedureProvider(QString name, QString processNameInit, int callDurationInit);

private slots:
    virtual void callRequestedSlot(const QByteArray& params, int callId) override;
    virtual void controlServerConnectedSlot() override;
    virtual void controlServerUnavailableErrorSlot() override;

private:
    int callDuration;
    int callCounter;
    QString processName;
};

#endif // TESTPROCEDUREHANDLER_H
