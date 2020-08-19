#include "testprocedurehandler.h"

TESTProcedureHandler::TESTProcedureHandler(QString name, QString processNameInit, int callDurationInit)
    : DIALOGProcedureHandler(name),
      callDuration(callDurationInit),
      callCounter(0),
      processName(processNameInit)
{

}

void TESTProcedureHandler::callRequestedSlot(QByteArray params, QString urlInit, int portInit)
{
    APIMessageLogger::getInstance().logProcedureCallReceived(getName(), params);

    QByteArray message;
    message.append(QString::number(callCounter));
    callCounter++;

    QThread::usleep(callDuration);

    emit callFinishedSignal(getName(), message, urlInit, portInit);

    std::cout << "Send procedure data: " << getName().toStdString() << " - " << QString(message).toStdString() << " to " << urlInit.toStdString() <<"|" << portInit << std::endl;
    APIMessageLogger::getInstance().logProcedureDataSent(getName(), params);
}
