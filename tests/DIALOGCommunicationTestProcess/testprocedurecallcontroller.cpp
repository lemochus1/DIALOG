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
                                                                  .tryCallProcedure(name, message);
    if (caller) {
        APIMessageLogger::GetInstance().logProcedureCallSent(name, message);
        std::cout << "Called procedure: " << name.toStdString() << std::endl;

        connect(caller.data(), &DIALOGNamedData::controlServerConnectedSignal,
                this, &TESTProcedureCallController::controlServerConnected);
        connect(caller.data(), &DIALOGNamedData::controlServerUnavailableErrorSignal,
                this, &TESTProcedureCallController::controlServerUnavailable);

        connect(caller.data(), &DIALOGProcedureCaller::responseReceivedSignal,
                this, &TESTProcedureCallController::responseReceived);
        connect(caller.data(), &DIALOGProcedureCaller::callFailedErrorSignal,
                this, &TESTProcedureCallController::callFailed);
        connect(caller.data(), &DIALOGProcedureCaller::procedureUnavailableErrorSignal,
                this, &TESTProcedureCallController::procedureUnavailable);
        connect(caller.data(), &DIALOGProcedureCaller::paramsInvalidErrorSignal,
                this, &TESTProcedureCallController::paramsInvalid);
        connect(caller.data(), &DIALOGProcedureCaller::timeoutPassedErrorSignal,
                this, &TESTProcedureCallController::timeoutPassed);

        waitForData(caller);
    }
    else {
        std::cout << "Procedure " << name.toStdString() << " could not be called" << std::endl;
    }
    callCounter++;
    if (callCounter < repeat) {
        timer->singleShot(duration, this, &TESTProcedureCallController::callProcedure);
    }
}

void TESTProcedureCallController::responseReceived(const QByteArray &message)
{
    std::cout << "Procedure caller: " << name.toStdString()
              << ": Response received." << std::endl;
}

void TESTProcedureCallController::controlServerConnected()
{
    std::cout << "Procedure caller: " << name.toStdString()
              << ": Control server connected." << std::endl;
}

void TESTProcedureCallController::callFailed(const QByteArray& message)
{
    std::cout << "Procedure caller: " << name.toStdString()
              << ": Call failed with ." << QString(message).toStdString() << std::endl;
}

void TESTProcedureCallController::controlServerUnavailable()
{
    std::cout << "Procedure caller: " << name.toStdString()
              << ": Control server lost." << std::endl;
    APIMessageLogger::GetInstance().logControlServerLost("procedure-caller", name);
}

void TESTProcedureCallController::procedureUnavailable()
{
    std::cout << "Procedure caller: " << name.toStdString()
              << ": Procedure unavailable." << std::endl;
    APIMessageLogger::GetInstance().logProcedureUnavailable(name);
}

void TESTProcedureCallController::paramsInvalid()
{
    std::cout << "Procedure caller: " << name.toStdString() << ": Params invalid." << std::endl;
}

void TESTProcedureCallController::timeoutPassed()
{
    std::cout << "Procedure caller: " << name.toStdString() << ": Timeout passed." << std::endl;
}

void TESTProcedureCallController::waitForData(QSharedPointer<DIALOGProcedureCaller> caller)
{
    bool ok = false;
    QByteArray data = caller->waitForResponse(ok);
    if (ok) {
        APIMessageLogger::GetInstance().logProcedureDataReceived(name, data);
        std::cout << "Waited for call and received: "
                  << name.toStdString()
                  << " - "
                  << QString(data).toStdString()
                  << std::endl;
    } else {
        std::cout << "No data received. Call ended with an error." << std::endl;
    }
}
