#ifndef TESTPROCEDUREHANDLER_H
#define TESTPROCEDUREHANDLER_H

#include <QObject>

#include "DIALOGapi.h"
#include "apimessagelogger.h"

class TESTProcedureHandler : public DIALOGProcedureHandler
{
    Q_OBJECT

public:
    TESTProcedureHandler(QString name, QString processNameInit, int callDurationInit);

public slots:
    void callRequestedSlot(QByteArray params, QString urlInit, int portInit) override;

private:
    int callDuration;
    int callCounter;
    QString processName;
};

#endif // TESTPROCEDUREHANDLER_H
