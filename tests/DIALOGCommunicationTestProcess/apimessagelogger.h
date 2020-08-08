#ifndef TESTAPIMESSAGELOGGER_H
#define TESTAPIMESSAGELOGGER_H

#include <QObject>
#include <QDebug>
#include <QDateTime>

class APIMessageLogger
{

public:
    static APIMessageLogger &getInstance();

    void logCommandRegistered(const QString& name, const QString& message);
    void logServiceRegistered(const QString& name, const QString& message);
    void logProcedureRegistered(const QString& name, const QString& message);

    void logCommandSent(const QString& name, const QString& message);
    void logProcedureCallSent(const QString& name, const QString& message);

    void logServiceRequested(const QString& name, const QString& message);

    void logServiceDataSent(const QString& name, const QString& message);
    void logProcedureDataSent(const QString& name, const QString& message);

    void logCommandReceived(const QString& name, const QString& message);
    void logServiceDataReceived(const QString& name, const QString& message);
    void logProcedureDataReceived(const QString& name, const QString& message);
    void logProcedureCallReceived(const QString& name, const QString& message);

private:
    APIMessageLogger();

    static APIMessageLogger* instance;

public:
    APIMessageLogger(APIMessageLogger const&) = delete;
    void operator=(APIMessageLogger const&) = delete;

private:
    QString composeLogMessage(const QString& action, const QString& type, const QString& name, const QString &message="");

    QString getDateTimeString();
    QString createStartElement(const QString& type, const QString& name="");
    QString createEndElement(const QString& type);
    QString createElement(const QString& type, const QString message, const QString& name="");
};

#endif // TESTAPIMESSAGELOGGER_H
