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
    message.append(processName + "_" + commandName + "_" + QString::number(sendCounter));
    if (sendCounter == repeat) {
        timer->stop();
    } else {
        sendCounter++;
    }
    mutex.unlock();

    if (toAddress) {
        process->sendDirectCommandSlot(commandName, message, address, port);
        qDebug() << "Send: " << QString(message) << " to " << address << "|" << port;
    } else if (toName) {
        process->sendDirectCommandSlot(commandName, message, targetProcessName);
        qDebug() << "Send: " << QString(message) << " to " << targetProcessName;
    } else {
        process->sendCommandSlot(commandName, message);
        qDebug() << "Send: " << QString(message) << " to all providers";
    }
}
