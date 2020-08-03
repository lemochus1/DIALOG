#ifndef THREADSAFEQMAP_H
#define THREADSAFEQMAP_H

#include <QMap>
#include <QSemaphore>
#include <QMutex>
#include "messagecontainer.h"

class QString;

class ThreadSafeQList
{
    QList<MessageContainer*> messages;
    QSemaphore semUsed;
    QMutex mutex;

public:
    ThreadSafeQList();
    void push(MessageContainer* messageContainer);
    MessageContainer* pop();
    bool isEmpty();
    int count();
};

#endif // THREADSAFEQMAP_H
