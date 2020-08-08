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
    message.append(process->getName() + "_" + name + "_" + QString::number(callCounter));

    DIALOGProcedureCaller* caller = process->callProcedure(name, message);
    APIMessageLogger::getInstance().logProcedureCallSent(name, message);
//    qDebug() << "Called: " << QString(message);
//    qDebug() << "Continuing...";
    bool ok = false;
    QByteArray data = caller->waitForData(ok);
    if (ok) {
        APIMessageLogger::getInstance().logProcedureDataReceived(name, message);
//        qDebug() << "Wait and received: " << QString(data);
    } else {
        //logovani chyb zatim nenam... ale je to api takze by to chtelo...
//        qDebug() << "No data received. Call ended in timeout.";
    }

    delete caller;

    callCounter++;
    if (callCounter < repeat) {
        timer->singleShot(duration*1000, this, &TESTProcedureCallController::callProcedure);
    }
}
