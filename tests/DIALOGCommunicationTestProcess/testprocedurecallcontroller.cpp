#include "testprocedurecallcontroller.h"

TESTProcedureCallController::TESTProcedureCallController(QString nameInit, DIALOGProcess* processInit, QString targetProcessInit, int durationInit, int repeatInit)
    : QObject(nullptr),
      name(nameInit),
      targetProcess(targetProcessInit),
      duration(durationInit),
      repeat(repeatInit),
      callCounter(0)
{
    process = processInit;
}

void TESTProcedureCallController::start()
{
    timer = new QTimer(this);
    timer->singleShot(duration*1000, this, &TESTProcedureCallController::callProcedure);
}

void TESTProcedureCallController::callProcedure()
{
    QByteArray message;
    message.append(QString::number(callCounter));

    DIALOGProcedureCaller* caller = process->callProcedure(name, message);
    APIMessageLogger::getInstance().logProcedureCallSent(name, message);
    std::cout << "Called procedure: " << name.toStdString() << std::endl;
    bool ok = false;
    QByteArray data = caller->waitForData(ok);
    if (ok) {
        APIMessageLogger::getInstance().logProcedureDataReceived(name, message);
        std::cout << "Waited for call and received: " << name.toStdString() << " - " << QString(data).toStdString() << std::endl;
    } else {
        //logovani chyb zatim nenam... ale je to api takze by to chtelo...
        std::cout << "No data received. Call ended by timeout: " << name.toStdString() << std::endl;
    }

    delete caller;

    callCounter++;
    if (callCounter < repeat) {
        timer->singleShot(duration*1000, this, &TESTProcedureCallController::callProcedure);
    }
}
