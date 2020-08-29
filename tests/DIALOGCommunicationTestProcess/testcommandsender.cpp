#include "testcommandsender.h"

TESTCommandSender::TESTCommandSender(QString nameInit,
                                     int pauseInit,
                                     int repeatInit,
                                     int messageSizeInit)
    : QObject(nullptr),
      commandName(nameInit),
      toName(false),
      toAddress(false),
      repeat(repeatInit),
      pause(pauseInit),
      size(messageSizeInit),
      sendCounter(1)
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
    QByteArray message = generateMessage();
    QString asString = APIMessageLogger::GetInstance().getMessageLogString(message);

    bool success = false;
    QString outputMessage = QString("Send command: %1 - %2")
                            .arg(commandName)
                            .arg(asString);
    if (toAddress) {
        success = DIALOGProcess::GetInstance().trySendCommand(commandName,
                                                              message,
                                                              address,
                                                              port);
        outputMessage += " to " + address + "|" + QString::number(port);
    } else if (toName) {
        success = DIALOGProcess::GetInstance().trySendCommand(commandName,
                                                              message,
                                                              targetProcessName);
        outputMessage += " to " + targetProcessName;
    } else {
        success = DIALOGProcess::GetInstance().trySendCommand(commandName, message);
    }

    if (success) {
        std::cout << outputMessage.toStdString() << std::endl;
        APIMessageLogger::GetInstance().logCommandSent(commandName, asString);
    } else {
        std::cout << "Command " << commandName.toStdString()
                  << " could not be send." << std::endl;
    }
}

QByteArray TESTCommandSender::generateMessage()
{
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
    return message;
}
