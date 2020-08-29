#ifndef TESTPROCESSCONTROLLER_H
#define TESTPROCESSCONTROLLER_H

#include <QObject>
#include <QPair>

#include "dialogapi.h"

#include "testcommandhandler.h"
#include "testcommandsender.h"
#include "testprocedurecallcontroller.h"
#include "testprocedureprovider.h"
#include "testservicesubscriber.h"
#include "testservicepublisher.h"
#include "apimessagelogger.h"

class TESTProcessController
{

public:
    TESTProcessController();

    bool setupProcess(const QString& xmlConfiguration);
    void startProcess();
    void startSenders();

private:
    bool readConfig(QXmlStreamReader& reader);
    bool readProcessElement(QXmlStreamReader& reader);
    bool readControlServerElement(QXmlStreamReader& reader);
    bool readRegisterElement(QXmlStreamReader& reader);
    bool readRequestElement(QXmlStreamReader& reader);
    bool readSendElement(QXmlStreamReader& reader);

    int tryGetIntAttribute(const QString& name,
                           const QXmlStreamAttributes& attributes,
                           int defaultValue);

    QList<QSharedPointer<DIALOGCommandHandler>> commandHandlers;
    QList<QSharedPointer<TESTServicePublisher>> ServicePublishers;
    QList<QSharedPointer<TESTServiceSubscriber>> serviceSubscribers;
    QList<QSharedPointer<TESTProcedureProvider>> procedureHandlers;
    QList<QSharedPointer<TESTCommandSender>> commandSenders;
    QMap<QString, QSharedPointer<TESTProcedureCallController>> procedureCallers;

};

#endif // TESTPROCESSCONTROLLER_H
