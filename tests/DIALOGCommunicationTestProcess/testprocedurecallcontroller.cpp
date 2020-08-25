#include "testprocedurecallcontroller.h"

TESTProcedureCallController::TESTProcedureCallController(QString nameInit, QString targetProcessInit, int durationInit, int repeatInit)
    : QObject(nullptr),
      name(nameInit),
      targetProcess(targetProcessInit),
      duration(durationInit),
      repeat(repeatInit),
      callCounter(0)
{
}

void TESTProcedureCallController::start()
{
    timer = new QTimer(this);
    timer->singleShot(duration, this, &TESTProcedureCallController::callProcedure);
}

void TESTProcedureCallController::callProcedure()
{
    QByteArray message;
    message.append(QString::number(callCounter));

    QSharedPointer<DIALOGProcedureCaller> caller = DIALOGProcess::GetInstance()
                                                                      .callProcedure(name, message);
    if (caller) {
        APIMessageLogger::GetInstance().logProcedureCallSent(name, message);
        std::cout << "Called procedure: " << name.toStdString() << std::endl;
        bool ok = false;
        QByteArray data = caller->waitForData(ok);
        if (ok) {
            APIMessageLogger::GetInstance().logProcedureDataReceived(name, data);
            std::cout << "Waited for call and received: "
                      << name.toStdString()
                      << " - "
                      << QString(data).toStdString()
                      << std::endl;
        } else {
            //logovani chyb zatim nenam... ale je to api takze by to chtelo...
            std::cout << "No data received. Call ended by timeout: "
                      << name.toStdString()
                      << std::endl;
        }

    }
    else {
        std::cout << "Error while calling procedure: " << name.toStdString() << std::endl;
    }
    callCounter++;
    if (callCounter < repeat) {
        timer->singleShot(duration, this, &TESTProcedureCallController::callProcedure);
    }
}
