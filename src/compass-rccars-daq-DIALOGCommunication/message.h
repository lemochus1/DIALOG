#ifndef MESSAGE_H
#define MESSAGE_H

#include "process.h"

class Message
{
public:
    explicit Message(Process* receiverProcessInit, QByteArray* headerInit, QByteArray* messageInit);
    ~Message();
    Process* receiverProcess;
    QByteArray* header;
    QByteArray* message;
};

#endif // MESSAGE_H
