#ifndef TESTPROCESSCONTROLLER_H
#define TESTPROCESSCONTROLLER_H

#include <QObject>
#include <QPair>

#include "DIALOGapi.h"

#include "testcommandhandler.h"
#include "testcommandsender.h"
#include "testprocedurecallcontroller.h"
#include "testprocedurehandler.h"
#include "testserviceprovider.h"
#include "testservicesubscriber.h"
#include "apimessagelogger.h"

class TESTProcessController : public QObject
{
    Q_OBJECT

    DIALOGProcess* process;

public:
    explicit TESTProcessController(QObject *parent = nullptr);

    bool setupProcess(const QString& xmlConfiguration);
    void startProcess();
    void startSenders();

    QString getProcessName() const;

private:
    bool readConfig(QXmlStreamReader& reader);
    bool readProcessElement(QXmlStreamReader& reader);
    bool readControlServerElement(QXmlStreamReader& reader);
    bool readRegisterElement(QXmlStreamReader& reader);
    bool readRequireElement(QXmlStreamReader& reader);
    bool readSendElement(QXmlStreamReader& reader);

    int tryGetIntAttribute(const QString& name, const QXmlStreamAttributes& attributes, int defaultValue);

    QList<TESTCommandHandler*> commandHandlers;
    QList<TESTServicePublisher*> servicePublishers;
    QList<TESTServiceSubscriber*> serviceSubscribers;
    QList<TESTProcedureHandler*> procedureHandlers;
    QList<TESTCommandSender*> commandSenders;//jak musi byt v cizim vlakne???
    QMap<QString, TESTProcedureCallController*> procedureCallers;

public slots:


signals:

};

#endif // TESTPROCESSCONTROLLER_H
