#include "testprocesscontroller.h"
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>

const QString PROCESS_ELEMENT = "process";
const QString CONTROL_SERVER_ELEMENT = "control-server";
const QString REGISTER_ELEMENT = "register";
const QString REQUEST_ELEMENT = "request";
const QString SEND_ELEMENT = "send";
const QString COMMAND_ELEMENT = "command";
const QString DIRECT_COMMAND_ELEMENT = "direct-command";
const QString PROCEDURE_ELEMENT = "procedure";
const QString SERVICE_ELEMENT = "service";

const QString NAME_ATTRIBUTE = "name";
const QString DURATION_ATTRIBUTE = "duration";
const QString REPEAT_ATTRIBUTE = "repeat";
const QString SIZE_ATTRIBUTE = "size";
const QString PORT_ATTRIBUTE = "port";
const QString ADDRESS_ATTRIBUTE = "address";
const QString PROCESS_ATTRIBUTE = "process";

TESTProcessController::TESTProcessController()
{
}

bool TESTProcessController::setupProcess(const QString& xmlConfiguration)
{
    QXmlStreamReader reader(xmlConfiguration);
    bool success = readConfig(reader);
    reader.clear();
    return success;
}

void TESTProcessController::startProcess()
{
    std::cout << "Started process: "
              << DIALOGProcess::GetInstance().getName().toStdString()
              << std::endl;
    DIALOGProcess::GetInstance().start();
}

void TESTProcessController::startSenders()
{
    for (auto sender : commandSenders) {
        sender->start();
    }
    for (auto publisher : servicePublishers) {
        publisher->start();
    }
    for (auto caller : procedureCallers) {
        caller->start();
    }
}

bool TESTProcessController::readConfig(QXmlStreamReader &reader)
{
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() != QXmlStreamReader::StartElement) {
            continue;
        }
        QStringRef elementName = reader.name();

        if (PROCESS_ELEMENT == elementName) {
            if(!readProcessElement(reader)) {
                return false;
            }
        } else if (CONTROL_SERVER_ELEMENT == elementName) {
            if(!readControlServerElement(reader)) {
                return false;
            }
        } else if (REGISTER_ELEMENT == elementName) {
            if(!readRegisterElement(reader)) {
                return false;
            }
        } else if (REQUEST_ELEMENT == elementName) {
            if(!readRequestElement(reader)) {
                return false;
            }
        } else if (SEND_ELEMENT == elementName) {
            if(!readSendElement(reader)) {
                return false;
            }
        } else {
            qWarning() << "Unkown element" << elementName << "parsed in config file.";
        }
    }
    if (reader.hasError()) {
        qWarning() << "Parse error ecurred while reading config file.";
        return false;
    }
    return true;
}

bool TESTProcessController::readProcessElement(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    if (!attributes.hasAttribute(NAME_ATTRIBUTE)){
        qWarning() << "Process name is not set in config.";
        return false;
    }
    DIALOGProcess::GetInstance().setName(attributes.value(NAME_ATTRIBUTE).toString());
    if (attributes.hasAttribute(PORT_ATTRIBUTE) && attributes.hasAttribute(ADDRESS_ATTRIBUTE)) {
        QString address = attributes.value(ADDRESS_ATTRIBUTE).toString();
        bool isInt = false;
        int port = attributes.value(PORT_ATTRIBUTE).toInt(&isInt);
        if (!isInt) {
            qWarning() << "Process port is not integer.";
            return false;
        }
//        process->setAddress(address, port);
    }
    return true;
}

bool TESTProcessController::readControlServerElement(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    if (attributes.hasAttribute(PORT_ATTRIBUTE) && attributes.hasAttribute(ADDRESS_ATTRIBUTE)) {
        QString address = attributes.value(ADDRESS_ATTRIBUTE).toString();
        bool isInt = false;
        int port = attributes.value(PORT_ATTRIBUTE).toInt(&isInt);
        if (isInt) {
            DIALOGProcess::GetInstance().setControlServerAddress(address, port);
            return true;
        }
    }
    qWarning() << "Control server element does not set its address and port right.";
    return false;
}

bool TESTProcessController::readRegisterElement(QXmlStreamReader &reader)
{
    while(!(reader.readNext() == QXmlStreamReader::EndElement
            && reader.name() == REGISTER_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }

        QStringRef elementName = reader.name();
        QXmlStreamAttributes attributes = reader.attributes();
        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qWarning() << "XML parsing error: unexpected element.";
            return false;
        }

        if (COMMAND_ELEMENT == elementName) {
            QSharedPointer<TESTCommandHandler> command =
                    QSharedPointer<TESTCommandHandler>(new TESTCommandHandler(elementText));
                    DIALOGProcess::GetInstance().registerCommand(command);
            commandHandlers.append(command);
            APIMessageLogger::GetInstance().logCommandRegistered(elementText);

        } else if (SERVICE_ELEMENT == elementName) {
            int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);
            int repeat = tryGetIntAttribute(REPEAT_ATTRIBUTE, attributes, 100);
            int size = tryGetIntAttribute(SIZE_ATTRIBUTE, attributes, 0);

            QSharedPointer<TESTServicePublisher> publisher =
                    QSharedPointer<TESTServicePublisher>(new TESTServicePublisher(
                                                             elementText,
                                                             DIALOGProcess::GetInstance().getName(),
                                                             duration,
                                                             repeat,
                                                             size));
            DIALOGProcess::GetInstance().registerService(publisher);
            servicePublishers.append(publisher);
            APIMessageLogger::GetInstance().logServiceRegistered(elementText);

        } else if (PROCEDURE_ELEMENT == elementName) {
            int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);
            QSharedPointer<TESTProcedurePublisher> publisher =
                    QSharedPointer<TESTProcedurePublisher>(
                                new TESTProcedurePublisher(elementText,
                                                           DIALOGProcess::GetInstance().getName(),
                                                           duration));
            DIALOGProcess::GetInstance().registerProcedure(publisher);
            procedureHandlers.append(publisher);
            APIMessageLogger::GetInstance().logProcedureRegistered(elementText);

        } else {
            qWarning() << "Unkown element" << elementName << "parsed in Register element.";
        }
    }
    return true;
}

bool TESTProcessController::readRequestElement(QXmlStreamReader &reader)
{
    while(!(reader.readNext() == QXmlStreamReader::EndElement
            && reader.name() == REQUEST_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }

        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qWarning() << "XML parsing error: unexpected element.";
            return false;
        }

        if (SERVICE_ELEMENT == reader.name()) {
            QSharedPointer<TESTServiceSubscriber> subscriber =
                    QSharedPointer<TESTServiceSubscriber>(new TESTServiceSubscriber(elementText));
            DIALOGProcess::GetInstance().requestService(subscriber);
            serviceSubscribers.append(subscriber);
            APIMessageLogger::GetInstance().logServiceRequested(elementText);
        } else {
            qWarning() << "Unkown element" << reader.name() << "parsed in Require element.";
        }
    }
    return true;
}

bool TESTProcessController::readSendElement(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();

    int repeat = tryGetIntAttribute(REPEAT_ATTRIBUTE, attributes, 1);
    int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);
    int size = tryGetIntAttribute(SIZE_ATTRIBUTE, attributes, 0);

    while(!(reader.readNext() == QXmlStreamReader::EndElement
            && reader.name() == SEND_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        QStringRef elementName = reader.name();
        attributes = reader.attributes();
        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qWarning() << "XML parsing error: unexpected element.";
            return false;
        }

        if (COMMAND_ELEMENT == elementName || DIRECT_COMMAND_ELEMENT == elementName) {
            QSharedPointer<TESTCommandSender> sender =
                    QSharedPointer<TESTCommandSender>(new TESTCommandSender(elementText,
                                                                            duration,
                                                                            repeat,
                                                                            size));
            if (DIRECT_COMMAND_ELEMENT == elementName) {
                if (attributes.hasAttribute(PORT_ATTRIBUTE)
                    && attributes.hasAttribute(ADDRESS_ATTRIBUTE)){
                    int port = tryGetIntAttribute(PORT_ATTRIBUTE, attributes, 8081);
                    QString address = attributes.value(ADDRESS_ATTRIBUTE).toString();
                    sender->setTargetAddress(address, port);

                } else if (attributes.hasAttribute(PROCESS_ATTRIBUTE)) {
                    sender->setTargetAddress(attributes.value(PROCESS_ATTRIBUTE).toString());
                } else {
                    qWarning() << "Unkown element" << elementName << "parsed in Send element.";
                    return false;
                }
            }
            commandSenders.append(sender);
        } else if (PROCEDURE_ELEMENT == elementName) {
            QSharedPointer<TESTProcedureCallController> caller =
                    QSharedPointer<TESTProcedureCallController>(
                                        new TESTProcedureCallController(elementText,
                                                                        "",
                                                                        duration,
                                                                        repeat));
            procedureCallers[elementText] = caller;
        } else {
            qWarning() << "Unkown element" << elementName << "parsed in Send element.";
        }
    }
    return true;
}

int TESTProcessController::tryGetIntAttribute(const QString &name,
                                              const QXmlStreamAttributes &attributes,
                                              int defaultValue)
{
    if (attributes.hasAttribute(name)) {
        bool isInt = false;
        int value = attributes.value(name).toInt(&isInt);
        if (isInt) {
            return value;
        }
        qDebug() << "So called int attribute is not integer.";
    }
    return defaultValue;
}
