#include "testprocesscontroller.h"
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>

const QString PROCESS_ELEMENT = "process";
const QString CONTROL_SERVER_ELEMENT = "control-server";
const QString REGISTER_ELEMENT = "register";
const QString REQUIRE_ELEMENT = "require";
const QString SEND_ELEMENT = "send";
const QString COMMAND_ELEMENT = "command";
const QString DIRECT_COMMAND_ELEMENT = "direct-command";
const QString PROCEDURE_ELEMENT = "procedure";
const QString SERVICE_ELEMENT = "service";

const QString NAME_ATTRIBUTE = "name";
const QString DURATION_ATTRIBUTE = "duration";
const QString REPEAT_ATTRIBUTE = "repeat";
const QString PORT_ATTRIBUTE = "port";
const QString ADDRESS_ATTRIBUTE = "address";
const QString PROCESS_ATTRIBUTE = "process";

TESTProcessController::TESTProcessController(QObject *parent) : QObject(parent)
{
    process = nullptr;
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
    std::cout << "Started process: " << process->getName().toStdString() << std::endl;
    process->start();
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

QString TESTProcessController::getProcessName() const
{
    return process->getName();
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
        } else if (REQUIRE_ELEMENT == elementName) {
            if(!readRequireElement(reader)) {
                return false;
            }
        } else if (SEND_ELEMENT == elementName) {
            if(!readSendElement(reader)) {
                return false;
            }
        } else {
            //Chci logovat chyby, ale ne takhle...
            qDebug() << "Unkown element '" << elementName << "' parsed in config file.";
        }
    }
    if (reader.hasError()) {
        qDebug() << "Parse error ecurred while reading config file.";
        return false;
    }
    return true;
}

bool TESTProcessController::readProcessElement(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();
    if (!attributes.hasAttribute(NAME_ATTRIBUTE)){
        qDebug() << "Process name is not set in config.";
        return false;
    }

    process = new DIALOGProcess(attributes.value(NAME_ATTRIBUTE).toString());
    if (attributes.hasAttribute(PORT_ATTRIBUTE) && attributes.hasAttribute(ADDRESS_ATTRIBUTE)) {
        QString address = attributes.value(ADDRESS_ATTRIBUTE).toString();
        bool isInt = false;
        int port = attributes.value(PORT_ATTRIBUTE).toInt(&isInt);
        if (!isInt) {
            qDebug() << "Process port is not integer.";
            return false;
        }
//        process->setAddress(address, port);
    }
    connect(process, &DIALOGProcess::serverDestroyed, QCoreApplication::instance(), &QCoreApplication::quit);
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
            process->setControlServerAdress(address, port);
            return true;
        }
    }
    qDebug() << "Control server element does not set its address and port right.";
    return false;
}

bool TESTProcessController::readRegisterElement(QXmlStreamReader &reader)
{
    while(!(reader.readNext() == QXmlStreamReader::EndElement && reader.name() == REGISTER_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }

        QStringRef elementName = reader.name();
        QXmlStreamAttributes attributes = reader.attributes();
        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qDebug() << "XML parsing error: unexpected element.";
            return false;
        }

        if (COMMAND_ELEMENT == elementName) {
            TESTCommandHandler* handler = new TESTCommandHandler(elementText);
            process->registerCommand(handler);
            commandHandlers.append(handler);
            APIMessageLogger::getInstance().logCommandRegistered(elementText);
        } else if (SERVICE_ELEMENT == elementName) {
            int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);
            TESTServicePublisher* publisher = new TESTServicePublisher(elementText, process->getName(), duration);
            process->registerService(publisher);
            servicePublishers.append(publisher);
            APIMessageLogger::getInstance().logServiceRegistered(elementText);
        } else if (PROCEDURE_ELEMENT == elementName) {
            int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);
            TESTProcedureHandler* handler = new TESTProcedureHandler(elementText, process->getName(), duration);
            process->registerProcedure(handler);
            procedureHandlers.append(handler);
            APIMessageLogger::getInstance().logProcedureRegistered(elementText);
        } else {
            qDebug() << "Unkown element '" << elementName << "' parsed in Register element.";
        }
    }
    return true;
}

bool TESTProcessController::readRequireElement(QXmlStreamReader &reader)
{
    while(!(reader.readNext() == QXmlStreamReader::EndElement && reader.name() == REQUIRE_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }

        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qDebug() << "XML parsing error: unexpected element.";
            return false;
        }

        if (SERVICE_ELEMENT == reader.name()) {
            TESTServiceSubscriber* subscriber = new TESTServiceSubscriber(elementText);
            process->requestService(subscriber);
            serviceSubscribers.append(subscriber);
            APIMessageLogger::getInstance().logServiceRequested(elementText);
        } else {
            qDebug() << "Unkown element '" << reader.name() << "' parsed in Require element.";
        }
    }
    return true;
}

bool TESTProcessController::readSendElement(QXmlStreamReader &reader)
{
    QXmlStreamAttributes attributes = reader.attributes();

    int repeat = tryGetIntAttribute(REPEAT_ATTRIBUTE, attributes, 1);
    int duration = tryGetIntAttribute(DURATION_ATTRIBUTE, attributes, 1);

    while(!(reader.readNext() == QXmlStreamReader::EndElement && reader.name() == SEND_ELEMENT)) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        QStringRef elementName = reader.name();
        attributes = reader.attributes();
        QString elementText = reader.readElementText();
        if (reader.hasError()) {
            qDebug() << "XML parsing error: unexpected element.";
            return false;
        }

        if (COMMAND_ELEMENT == elementName) {
            TESTCommandSender* sender = new TESTCommandSender(elementText, process, duration, repeat);
            commandSenders.append(sender);
        } else if (DIRECT_COMMAND_ELEMENT == elementName) {
            TESTCommandSender* sender = new TESTCommandSender(elementText, process, duration, repeat);
            if (attributes.hasAttribute(PORT_ATTRIBUTE) && attributes.hasAttribute(ADDRESS_ATTRIBUTE)){
                int port = tryGetIntAttribute(PORT_ATTRIBUTE, attributes, 8081);
                QString address = attributes.value(ADDRESS_ATTRIBUTE).toString();
                sender->setTargetAddress(address, port);
            } else if (attributes.hasAttribute(PROCESS_ATTRIBUTE)) {
                sender->setTargetAddress(attributes.value(PROCESS_ATTRIBUTE).toString());
            } else {
                delete sender;
                qDebug() << "Unkown element '" << elementName << "' parsed in Send element.";
                return false;
            }
            commandSenders.append(sender);
        } else if (PROCEDURE_ELEMENT == elementName) {
            //Procedure chyby target process.... zatim to tak necham
            TESTProcedureCallController* caller = new TESTProcedureCallController(elementText, process, "", duration, repeat);
            procedureCallers[elementText] = caller;
        } else {
            qDebug() << "Unkown element '" << elementName << "' parsed in Send element.";
        }
    }
    return true;
}

int TESTProcessController::tryGetIntAttribute(const QString &name, const QXmlStreamAttributes &attributes, int defaultValue)
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
