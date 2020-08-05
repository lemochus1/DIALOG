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
    qDebug() << "Received call request with params: " << QString(params);

    QByteArray message;
    message.append(processName + "_" + getName() + "_" + QString::number(callCounter));
    callCounter++;

    QThread::sleep(callDuration);

    emit callFinishedSignal(getName(), message, urlInit, portInit);

    qDebug() << "Send: " << QString(message) << " to " << urlInit << "|" << portInit;
}
