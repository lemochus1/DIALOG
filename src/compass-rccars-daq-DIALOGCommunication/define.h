#ifndef DEFINE_H
#define DEFINE_H

#include <QString>
#include <QList>
#include <QDebug>
#include <QDateTime>

enum ProcessType { ControlServer = 1, Custom = 2, Monitoring = 3 };

#define HEARTBEAT_TIMER 50
#define HEARTBEAT_CHECKER_TIMER 500
#define RECONNECTION_TIMER 3000
#define IDLE_SOCKET_TIMER 1000
#define SEND_SOCKET_TIMER 1
#define HARD_STOP_TIMER 3000
#define MAXIMUM_OF_PENDING_RECEIVER_CONNECTIONS 512
#define MAXIMUM_OF_PENDING_SENDER_CONNECTIONS 512
#define NUMBER_OF_RECEIVER_THREADS 1
#define NUMBER_OF_SENDER_THREADS 1

#define SEPARATOR '|'
#define CONNECT_TO_CONTROL_SERVER "CONNECT_TO_CONTROL_SERVER"
#define SUCCESSFULY_CONNECTED "SUCCESSFULY_CONNECTED"
#define CONNECTION_LOST "CONNECTION_LOST"
#define REGISTER_SERVICE "REGISTER_SERVICE"
#define REQUEST_SERVICE "REQUEST_SERVICE"
#define INFO_SERVICE "INFO_SERVICE"
#define SUBSCRIBE_SERVICE "SUBSCRIBE_SERVICE"
#define UNSUBSCRIBE_SERVICE "UNSUBSCRIBE_SERVICE"
#define SERVICE_MESSAGE "SERVICE_MESSAGE"
#define LIST_OF_AVAILABLE_SERVICES "LIST_OF_AVAILABLE_SERVICES"
#define REGISTER_COMMAND "REGISTER_COMMAND"
#define UNREGISTER_COMMAND "UNREGISTER_COMMAND"
#define COMMAND_MESSAGE "COMMAND_MESSAGE"
#define DIRECT "DIRECT"
#define HEARTBEAT "HEARTBEAT"
#define LOST_SENDER "LOST_SENDER"
#define LOST_RECEIVER "LOST_RECEIVER"
#define INFO_MONITORING "INFO_MONITORING"

#define PROCEDURE_MESSAGE "PROCEDURE_MESSSAGE"
#define PROCEDURE_CALL "PROCEDURE_CALL"
#define PROCEDURE_DATA "PROCEDURE_DATA"
#define REGISTER_PROCEDURE "REGISTER_PROCEDURE"
#define UNREGISTER_PROCEDURE "UNREGISTER_PROCEDURE"
/* ERRORS */
#define DIALOG_ERROR "DIALOG_ERROR"

#define SERVICE_DUPLICATE "SERVICE_DUPLICATE"

#define CONNECTION_ERROR "CONNECTION_ERROR"
#define HOST_NOT_FOUND_ERROR "HOST_NOT_FOUND_ERROR"
#define CONNECTION_REFUSED_ERROR "CONNECTION_REFUSED_ERROR"
#define DEFAULT_ERROR "DEFAULT_ERROR"

/* COMMON FUNCTIONS */
class DIALOGCommon
{
public:
    static QByteArray section(QByteArray* array, quint16 part)
    {
        int index = 0;
        for (int i = 0; i < part; i++)
            index = array->indexOf(SEPARATOR, index) + 1;

        return array->mid(index);
    }

    static QString transformMessageToPlainText(QByteArray message)
    {
        QString transformedMessage;
        for(int i = 0; i < message.size(); i++)
        {
            QByteArray part = message.mid(i, 1);
            if (i < 4 || (i > 7 && i < 12) || (i > 15 && i < 20))
            {
                QByteArray toHex = part.toHex();
                transformedMessage.append("\\x" + QString::fromUtf8(toHex.data(), toHex.size()).toUpper());
            }
            else
            {
                transformedMessage.append(QString::fromUtf8(part.data(), part.size()));
            }
        }
        return transformedMessage;
    }

    static QByteArray transformMessageFromPlainText(QString message)
    {
        QByteArray transformedMessage;
        transformedMessage.append(QByteArray::fromHex(message.mid(0, 16).toUtf8()));
        transformedMessage.append(message.mid(16, 4));
        transformedMessage.append(QByteArray::fromHex(message.mid(20, 16).toUtf8()));
        transformedMessage.append(message.mid(36, 4));
        transformedMessage.append(QByteArray::fromHex(message.mid(40, 16).toUtf8()));
        transformedMessage.append(message.mid(56));
        return transformedMessage;
    }

    static void logMessage(const QString& message)
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
                 << message;
    }

    static void logMessage(const QStringList& message)
    {
        for (const auto& messagePart : message) {
            DIALOGCommon::logMessage(messagePart);
        }
    }
};

/* MESSAGE TYPES */
enum DIALOGMessageHandlerType {
    CommandHandler = 0,
    ServicePublisher = 1,
    ServiceSubscriber = 2,
    ProcedureProvider = 3,
    ProcedureCaller = 4
};

const QMap<DIALOGMessageHandlerType, QString> DIALOGMessageStrings {
                                    {DIALOGMessageHandlerType::CommandHandler, "Command"},
                                    {DIALOGMessageHandlerType::ServicePublisher, "Service"},
                                    {DIALOGMessageHandlerType::ServiceSubscriber, "Service"},
                                    {DIALOGMessageHandlerType::ProcedureProvider, "Procedure"},
                                    {DIALOGMessageHandlerType::ProcedureCaller, "Procedure"}};

enum DIALOGErrorCode {
    NoError = 0,
    LostControlServer = 1,
};


#endif // DEFINE_H
