#include "daemonthread.h"

DaemonThread::DaemonThread()
{
    mainThread = currentThread();

    moveToThread(this);
}

DaemonThread::~DaemonThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "DaemonThread destructor";
}

void DaemonThread::run()
{
    dialogControlServerAddress = QString(getenv("DIALOG_CONTROL_SERVER_ADDRESS"));
    dialogControlServerPort = QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt();
    dialogControlServerKey = dialogControlServerAddress + SEPARATOR + QString::number(dialogControlServerPort);
    dialogInfo = NULL;
    postTimer = new QTimer();
    jsonArray = new QJsonArray();

    QObject::connect(this, &DaemonThread::stopInitiated, this, &DaemonThread::stopDaemonThreadSlot);
    QObject::connect(postTimer, &QTimer::timeout, this, &DaemonThread::postTimerSlot);
    postTimer->start(POST_TIMER);

    daemonThreadEventLoop = new QEventLoop();
    daemonThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of DaemonThread EventLoop";
}

void DaemonThread::updateDIALOGInfoSlot(DIALOGInfo *dialogInfoInit, DIALOGInfo *dialogInfoOld)
{
    dialogInfo = dialogInfoInit;

    foreach(Service* service, dialogInfo->services)
    {
        Q_EMIT requestServiceSignal(service->serviceName);
    }

    foreach(Command* command, dialogInfo->commands)
    {
        Q_EMIT registerCommandSignal(command->commandName);
    }

    if(dialogInfoOld != NULL)
        delete dialogInfoOld;
}

void DaemonThread::serviceOutputSlot(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message)
{
    QString senderKey = senderAddress + SEPARATOR + QString::number(senderPort);

    if(dialogInfo->processes.contains(senderKey))
    {
        QList<QByteArray> headerList = header.split(SEPARATOR);
        if(dialogInfo->services.contains(headerList[1]))
        {
            Service* service = dialogInfo->services[headerList[1]];

            QJsonObject recordService;

            QJsonObject senderObject;
            senderObject.insert("SenderAddress", QJsonValue::fromVariant(senderAddress));
            senderObject.insert("SenderPort", QJsonValue::fromVariant(senderPort));
            senderObject.insert("SenderName", QJsonValue::fromVariant(dialogInfo->processes[senderKey]->processName));
            recordService.insert("Sender", senderObject);

            QJsonObject receiversObject;
            int i = 0;
            foreach(Process* receiver, service->receivers)
            {
                QJsonObject receiverObject;
                receiverObject.insert("ReceiverAddress", QJsonValue::fromVariant(receiver->processAddress));
                receiverObject.insert("ReceiverPort", QJsonValue::fromVariant(receiver->processPort));
                receiverObject.insert("ReceiverName", QJsonValue::fromVariant(receiver->processName));
                receiversObject.insert("Receiver" + i, receiverObject);
                i++;
            }
            recordService.insert("Receivers", receiversObject);

            recordService.insert("DateTime", QJsonValue::fromVariant(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ssZ")));
            recordService.insert("MessageHeader", QJsonValue::fromVariant(header));
            recordService.insert("MessageBody", QJsonValue::fromVariant(message.toBase64()));

            jsonArray->push_back(recordService);
        }
    }
}

void DaemonThread::commandOutputSlot(QString senderAddress, quint16 senderPort, QByteArray header, QByteArray message)
{
    QString senderKey = senderAddress + SEPARATOR + QString::number(senderPort);

    if(senderKey == dialogControlServerKey)
    {
        QList<QByteArray> headerList = header.split(SEPARATOR);
        if(dialogInfo->commands.contains(headerList[1]))
        {
            Command* command = dialogInfo->commands[headerList[1]];

            QJsonObject recordCommand;

            QJsonObject senderObject;
            senderObject.insert("SenderAddress", QJsonValue::fromVariant(dialogControlServerAddress));
            senderObject.insert("SenderPort", QJsonValue::fromVariant(dialogControlServerPort));
            senderObject.insert("SenderName", QJsonValue::fromVariant("ControlServer"));
            recordCommand.insert("Sender", senderObject);

            QJsonObject receiversObject;
            int i = 0;
            foreach(Process* receiver, command->receivers)
            {
                QJsonObject receiverObject;
                receiverObject.insert("ReceiverAddress", QJsonValue::fromVariant(receiver->processAddress));
                receiverObject.insert("ReceiverPort", QJsonValue::fromVariant(receiver->processPort));
                receiverObject.insert("ReceiverName", QJsonValue::fromVariant(receiver->processName));
                receiversObject.insert("Receiver" + i, receiverObject);
                i++;
            }
            recordCommand.insert("Receivers", receiversObject);

            recordCommand.insert("DateTime", QJsonValue::fromVariant(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ssZ")));
            recordCommand.insert("MessageHeader", QJsonValue::fromVariant(header));
            recordCommand.insert("MessageBody", QJsonValue::fromVariant(message.toBase64()));

            jsonArray->push_back(recordCommand);
        }
    }
}

void DaemonThread::postTimerSlot()
{
    QJsonDocument doc(*jsonArray);

    QUrl url(POST_URL);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this, &DaemonThread::postRequestFinishedSlot);
    connect(manager, &QNetworkAccessManager::finished, manager, &QObject::deleteLater);

    //qDebug() << doc.toJson();
    manager->post(request, doc.toJson());

    delete jsonArray;
    jsonArray = new QJsonArray();
}

void DaemonThread::postRequestFinishedSlot(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError)
    {
        QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
        if (!statusCode.isValid())
        return;

        int status = statusCode.toInt();

        if (status != 200)
        {
            QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
            qDebug() << status << ": " << reason;
        }
        /*else
            qDebug() << status;*/
    }
    else if(reply->error() == QNetworkReply::TimeoutError)
    {
        qDebug() << "No internet connection.";
    }
    else
    {
        qDebug() << reply->error();
    }
}

void DaemonThread::stop()
{
    Q_EMIT stopInitiated();
}

void DaemonThread::stopDaemonThreadSlot()
{
    postTimer->stop();
    delete postTimer;

    delete jsonArray;

    daemonThreadEventLoop->exit();
    moveToThread(mainThread);
}
