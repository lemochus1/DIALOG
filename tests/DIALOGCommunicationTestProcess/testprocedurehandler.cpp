#include "testprocedurehandler.h"

TESTProcedureProvider::TESTProcedureProvider(QString name,
                                           QString processNameInit,
                                           int callDurationInit)
    : DIALOGProcedureProvider(name),
      callDuration(callDurationInit),
      callCounter(0),
      processName(processNameInit)
{

}

void TESTProcedureProvider::callRequestedSlot(QByteArray params, QString addressInit, int portInit)
{
    APIMessageLogger::GetInstance().logProcedureCallReceived(getName(), params);

    QByteArray message;
    message.append(QString::number(callCounter));
    callCounter++;

    QThread::usleep(callDuration);

    emit callFinishedSignal(getName(), message, addressInit, portInit);

    std::cout << "Send procedure data: " << getName().toStdString()
              << " - " << QString(message).toStdString() << " to "
              << addressInit.toStdString() <<"|" << portInit << std::endl;
    APIMessageLogger::GetInstance().logProcedureDataSent(getName(), params);
}
