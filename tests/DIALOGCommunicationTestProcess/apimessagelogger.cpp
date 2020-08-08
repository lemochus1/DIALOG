#include "apimessagelogger.h"

const QString SENT = "sent";
const QString RECEIVED = "received";
const QString REGISTERED = "registered";
const QString REQUESTED= "requested";

const QString COMMAND = "command";
const QString PROCEDURE = "procedure";
const QString PROCEDURE_CALL = "procedure-call";
const QString PROCEDURE_DATA = "procedure-data";
const QString SERVICE = "service";
const QString SERVICE_DATA = "service-data";


APIMessageLogger &APIMessageLogger::getInstance()
{
    static APIMessageLogger instance;
    return instance;
}

void APIMessageLogger::logCommandRegistered(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(REGISTERED, COMMAND, name, message);
}

void APIMessageLogger::logServiceRegistered(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(REGISTERED, SERVICE, name, message);
}

void APIMessageLogger::logProcedureRegistered(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(REGISTERED, PROCEDURE, name, message);
}

void APIMessageLogger::logCommandSent(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(SENT, COMMAND, name, message);
}

void APIMessageLogger::logProcedureCallSent(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(SENT, PROCEDURE_CALL, name, message);
}

void APIMessageLogger::logServiceRequested(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(REQUESTED, SERVICE, name, message);
}

void APIMessageLogger::logServiceDataSent(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(SENT, SERVICE_DATA, name, message);
}

void APIMessageLogger::logProcedureDataSent(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(SENT, PROCEDURE_DATA, name, message);
}

void APIMessageLogger::logCommandReceived(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(RECEIVED, COMMAND, name, message);
}

void APIMessageLogger::logServiceDataReceived(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(RECEIVED, SERVICE_DATA, name, message);
}

void APIMessageLogger::logProcedureDataReceived(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(RECEIVED, PROCEDURE_DATA, name, message);
}

void APIMessageLogger::logProcedureCallReceived(const QString &name, const QString &message)
{
    qDebug() << composeLogMessage(RECEIVED, PROCEDURE_CALL, name, message);
}

APIMessageLogger::APIMessageLogger(){}

QString APIMessageLogger::composeLogMessage(const QString &action, const QString &type, const QString &name, const QString &message)
{
    return getDateTimeString() + " " + createElement("api", createElement(action, createElement(type, message, name)));
}

QString APIMessageLogger::getDateTimeString()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

QString APIMessageLogger::createStartElement(const QString &type, const QString &name)
{
    if (!name.isEmpty()){
        return "<" + type + " name='"+ name +"'>";
    }
    return "<" + type + ">";
}

QString APIMessageLogger::createEndElement(const QString &type)
{
    return "</" + type + ">";
}

QString APIMessageLogger::createElement(const QString &type, const QString message, const QString &name)
{
    return createStartElement(type, name) + message + createEndElement(type);
}
