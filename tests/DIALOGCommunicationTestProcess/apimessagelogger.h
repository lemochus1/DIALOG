#ifndef TESTAPIMESSAGELOGGER_H
#define TESTAPIMESSAGELOGGER_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFile>


class APIMessageLogger
{

public:
    static APIMessageLogger &GetInstance();

    void setLogFile(const QString& file);

    void logCommandRegistered(const QString& name);
    void logServiceRegistered(const QString& name);
    void logProcedureRegistered(const QString& name);

    void logCommandSent(const QString& name, const QString& message);
    void logProcedureCallSent(const QString& name, const QString& message);

    void logServiceRequested(const QString& name);

    void logServiceDataSent(const QString& name, const QString& message);
    void logProcedureDataSent(const QString& name, const QString& message);

    void logCommandReceived(const QString& name, const QString& message);
    void logServiceDataReceived(const QString& name, const QString& message);
    void logProcedureDataReceived(const QString& name, const QString& message);
    void logProcedureCallReceived(const QString& name, const QString& message);

    void logServiceUnavailable(const QString& name);
    void logProcedureUnavailable(const QString& name);

    void logControlServerLost(const QString objectType, const QString& name);

    QString getMessageLogString(const QByteArray& message);
    QString generateRandomString(int size);

private:
    APIMessageLogger();

public:
    APIMessageLogger(APIMessageLogger const&) = delete;
    void operator=(APIMessageLogger const&) = delete;

private:
    void printMessage(const QString& message);

    QString composeLogMessage(const QString& action, const QString& type, const QString& name, const QString &message="");

    QString getDateTimeString();
    QString createStartElement(const QString& type, const QString& name="");
    QString createEndElement(const QString& type);
    QString createElement(const QString& type, const QString message, const QString& name="");

    QString logFile;
};

#endif // TESTAPIMESSAGELOGGER_H
