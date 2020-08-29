#include "testprocedureprovider.h"

TESTProcedureProvider::TESTProcedureProvider(QString name,
                                           QString processNameInit,
                                           int callDurationInit)
    : DIALOGProcedureProvider(name),
      callDuration(callDurationInit),
      callCounter(0),
      processName(processNameInit)
{

}

void TESTProcedureProvider::callRequestedSlot(const QByteArray &params, int callId)
{
    APIMessageLogger::GetInstance().logProcedureCallReceived(getName(), params);

    QByteArray message;
    message.append(QString::number(callCounter));
    callCounter++;

    QThread::usleep(callDuration);

    callFinishedSlot(message, callId);

    std::cout << "Send procedure data: " << getName().toStdString()
              << " - " << QString(message).toStdString() << std::endl;

    APIMessageLogger::GetInstance().logProcedureDataSent(getName(), params);
}

void TESTProcedureProvider::controlServerConnectedSlot()
{
    std::cout << "Procedure provider: " << getName().toStdString()
              << ": Control server connected." << std::endl;
}

void TESTProcedureProvider::controlServerUnavailableErrorSlot()
{
    std::cout << "Procedure caller: " << getName().toStdString()
              << ": Control server lost." << std::endl;
    APIMessageLogger::GetInstance().logControlServerLost("procedure-provider", getName());
}
