#ifndef MESSAGECONTAINER_H
#define MESSAGECONTAINER_H

#include <QObject>

class MessageContainer
{
    QByteArray* header;
    QByteArray* message;
    quint32 messageCounter;

public:
    explicit MessageContainer(QByteArray* headerInit, QByteArray* messageInit = NULL, quint32 messageCounterInit = 1);
    ~MessageContainer();
    QByteArray* getHeader();
    QByteArray* getMessage();
    quint32 deleteMessage();
};

#endif // MESSAGECONTAINER_H
