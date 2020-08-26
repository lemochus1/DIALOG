#include "testcommandsender.h"

TESTCommandSender::TESTCommandSender(QString nameInit, int pauseInit, int repeatInit, int messageSizeInit)
    : QObject(nullptr),
      commandName(nameInit),
      pause(pauseInit),
      repeat(repeatInit),
      sendCounter(0),
      toName(false),
      toAddress(false),
      size(messageSizeInit)
{
    processName = DIALOGProcess::GetInstance().getName();
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
    timer->start(pause);
}

void TESTCommandSender::sendCommand()
{
    mutex.lock();
    QByteArray message;
    message.append(QString::number(sendCounter));
    if (size > 0) {
        message.append("-" + QString::number(size) + "-");
        message.append(APIMessageLogger::GetInstance().generateRandomString(size));
    }
    if (sendCounter == repeat) {
        timer->stop();
    } else {
        sendCounter++;
    }
    mutex.unlock();

    QString asString = APIMessageLogger::GetInstance().getMessageLogString(message);

    if (toAddress) {
        DIALOGProcess::GetInstance().trySendDirectCommand(commandName, message, address, port);
        std::cout << "Send command: " << commandName.toStdString()
                  << " - " << asString.toStdString() << " to "
                  << address.toStdString() << "|"
                  << port << std::endl;
    } else if (toName) {
        DIALOGProcess::GetInstance().trySendDirectCommand(commandName, message, targetProcessName);
        std::cout << "Send command: " << commandName.toStdString()
                  << " - " << asString.toStdString() << std::endl;
    } else {
        DIALOGProcess::GetInstance().trySendCommand(commandName, message);
        std::cout << "Send command: " << commandName.toStdString()
                  << " - " << asString.toStdString() << std::endl;
    }
    APIMessageLogger::GetInstance().logCommandSent(commandName, asString);
}
