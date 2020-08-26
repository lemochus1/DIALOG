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

public slots:
    void callRequestedSlot(QByteArray params, QString urlInit, int portInit) override;

private:
    int callDuration;
    int callCounter;
    QString processName;
};

#endif // TESTPROCEDUREHANDLER_H
