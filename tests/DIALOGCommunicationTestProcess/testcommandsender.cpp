#include "testcommandsender.h"

TESTCommandSender::TESTCommandSender(QString nameInit, DIALOGProcess *processInit, int pauseInit, int repeatInit)
    : QObject(nullptr),
      commandName(nameInit),
      pause(pauseInit),
      repeat(repeatInit),
      sendCounter(0),
      toName(false),
      toAddress(false)
{
    process = processInit;
    processName = process->getName();
}

void TESTCommandSender::setTargetAddress(QString processNameInit)
{
    targetProcessName = processNameInit;
    toAddress = false;
    toName = true;
}

void TESTCommandSender::setTargetAddress(QString addressInit, int portInit)
{
    address = addressInit;
    port = portInit;
    toAddress = true;
    toName = false;
}

void TESTCommandSender::start()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TESTCommandSender::sendCommand);
    timer->start(pause * 1000);
}

void TESTCommandSender::sendCommand()
{
    mutex.lock();
    QByteArray message;
    message.append(QString::number(sendCounter));
    if (sendCounter == repeat) {
        timer->stop();
    } else {
        sendCounter++;
    }
    mutex.unlock();

    if (toAddress) {
        process->sendDirectCommandSlot(commandName, message, address, port);
        std::cout << "Send command: " << commandName.toStdString() << " - " << QString(message).toStdString() << " to " << address.toStdString() << "|" << port << std::endl;
    } else if (toName) {
        process->sendDirectCommandSlot(commandName, message, targetProcessName);
        std::cout << "Send command: " << commandName.toStdString() << " - " << QString(message).toStdString() << std::endl;
    } else {
        process->sendCommandSlot(commandName, message);
        std::cout << "Send command: " << commandName.toStdString() << " - " << QString(message).toStdString() << std::endl;
    }
    APIMessageLogger::getInstance().logCommandSent(commandName, message);
}
