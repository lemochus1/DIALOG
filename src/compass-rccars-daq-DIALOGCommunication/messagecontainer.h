#ifndef MESSAGECONTAINER_H
#define MESSAGECONTAINER_H

#include <QObject>

class MessageContainer
{
    QByteArray* header;
    QByteArray* message;
    quint32 messageCounter;

public:
    explicit MessageContainer(QByteArray* headerInit,
                              QByteArray* messageInit = nullptr,
                              quint32 messageCounterInit = 1);
    ~MessageContainer();

    QByteArray* getHeader();
    QByteArray* getMessage();
    quint32 deleteMessage();
    quint32 getSize();
};

#endif // MESSAGECONTAINER_H
