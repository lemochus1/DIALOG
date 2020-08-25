#include "messagecontainer.h"

MessageContainer::MessageContainer(QByteArray* headerInit,
                                   QByteArray* messageInit,
                                   quint32 messageCounterInit)
{
    header = headerInit;
    message = messageInit;
    messageCounter = messageCounterInit;
}

MessageContainer::~MessageContainer()
{
    if (message)
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
    if (messageCounter > 0)
        messageCounter--;
    return messageCounter;
}

quint32 MessageContainer::getSize()
{
    quint32 messageSize = 0;
    if (header)
    {
        messageSize += header->size();
    }
    if (message)
    {
        messageSize += message->size();
    }
    return messageSize;
}
