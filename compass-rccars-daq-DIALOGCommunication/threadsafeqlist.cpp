#include "threadsafeqlist.h"
#include <iostream>

ThreadSafeQList::ThreadSafeQList() : semUsed(0)
{
}

void ThreadSafeQList::push(MessageContainer* messageContainer)
{
    mutex.lock();
    messages.append(messageContainer);
    mutex.unlock();
    semUsed.release(1);
}

MessageContainer* ThreadSafeQList::pop()
{
    semUsed.acquire(1);
    mutex.lock();
    MessageContainer* message = messages.first();
    messages.removeFirst();
    mutex.unlock();
    return message;
}

bool ThreadSafeQList::isEmpty()
{
    return semUsed.available() == 0;
}

int ThreadSafeQList::count()
{
    return semUsed.available();
}
