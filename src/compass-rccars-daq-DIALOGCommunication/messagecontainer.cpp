#include "messagecontainer.h"

MessageContainer::MessageContainer(QByteArray* headerInit, QByteArray* messageInit, quint32 messageCounterInit)
{
    header = headerInit;
    message = messageInit;
    messageCounter = messageCounterInit;
}

MessageContainer::~MessageContainer()
{
    if(message != NULL)
        delete message;
}

QByteArray* MessageContainer::getHeader()
{
    return header;
}

QByteArray* MessageContainer::getMessage()
{
    return message;
}

quint32 MessageContainer::deleteMessage()
{
    if(messageCounter > 0)
        messageCounter--;

    return messageCounter;
}
