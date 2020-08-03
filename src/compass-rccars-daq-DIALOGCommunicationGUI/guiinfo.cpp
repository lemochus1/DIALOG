#include "guiinfo.h"

GUIInfo::GUIInfo(QObject *parent) :
    QObject(parent)
{
}

void GUIInfo::readGUIInfo(QString info)
{
    XMLReader.clear();
    XMLReader.addData(info);
    while(!XMLReader.atEnd())
    {
        if(XMLReader.isStartElement())
        {
            if(XMLReader.name() == "processes")
            {
                readGUIInfoProcessElement();
            }
            else
            {
                XMLReader.raiseError(QObject::tr("The CommunicationControlServer is sending info in a wrong format."));
            }
        }
        else
        {
            XMLReader.readNext();
        }
    }
}

void GUIInfo::readGUIInfoProcessElement()
{
    XMLReader.readNext();
    while (!XMLReader.atEnd())
    {
        if (XMLReader.isEndElement())
        {
            XMLReader.readNext();
            break;
        }

        if (XMLReader.isStartElement())
        {
            if (XMLReader.name() == "process")
            {
                QXmlStreamAttributes atr = XMLReader.attributes();
                QString processKey = atr.value("address").toString() + SEPARATOR + atr.value("port").toString();
                if(!processes.contains(processKey))
                {
                    Process* process = new Process(atr.value("address").toString(), atr.value("port").toString().toUShort(), Custom, atr.value("name").toString(), atr.value("pid").toString().toULongLong());
                    addProcess(process);
                }
                readGUIInfoServiceOrCommandElement(processes[processKey]);
            }
            else
            {
                skipUnknownElement();
            }
        }
        else
        {
            XMLReader.readNext();
        }
    }
}

void GUIInfo::readGUIInfoServiceOrCommandElement(Process* process)
{
    XMLReader.readNext();
    while (!XMLReader.atEnd())
    {
        if (XMLReader.isEndElement())
        {
            XMLReader.readNext();
            break;
        }

        if (XMLReader.isStartElement())
        {
            if (XMLReader.name() == "service")
            {
                QXmlStreamAttributes atr = XMLReader.attributes();
                Service* service = new Service(atr.value("name").toString());
                service->addSender(process);
                process->addServiceAsSender(service);
                addService(service);
                readGUIInfoReceiverElement(service);
            }
            else if (XMLReader.name() == "command")
            {
                QXmlStreamAttributes atr = XMLReader.attributes();
                QString commandName = atr.value("name").toString();
                //qDebug() << commandName;
                if(!commands.contains(commandName))
                {
                    Command* command = new Command(commandName);
                    addCommand(command);
                }
                commands[commandName]->addReceiver(process);
                process->addCommand(commands[commandName]);
                XMLReader.readNext();
                XMLReader.readNext();
            }
            else
            {
                skipUnknownElement();
            }
        }
        else
        {
            XMLReader.readNext();
        }
    }
}

void GUIInfo::readGUIInfoReceiverElement(Service* service)
{
    XMLReader.readNext();
    while (!XMLReader.atEnd())
    {
        if (XMLReader.isEndElement())
        {
            XMLReader.readNext();
            break;
        }

        if (XMLReader.isStartElement())
        {
            if (XMLReader.name() == "receiver")
            {
                QXmlStreamAttributes atr = XMLReader.attributes();
                QString processKey = atr.value("address").toString() + SEPARATOR + atr.value("port").toString();
                if(!processes.contains(processKey))
                {
                    Process* process = new Process(atr.value("address").toString(), atr.value("port").toString().toUShort(), Custom, atr.value("name").toString(), atr.value("pid").toString().toULongLong());
                    addProcess(process);
                }
                service->addReceiver(processes[processKey]);
                processes[processKey]->addServiceAsReceiver(service);
                XMLReader.readNext();
                XMLReader.readNext();
            }
            else
            {
                skipUnknownElement();
            }
        }
        else
        {
            XMLReader.readNext();
        }
    }
}

void GUIInfo::skipUnknownElement()
{
    XMLReader.readNext();
    while (!XMLReader.atEnd())
    {
        if (XMLReader.isEndElement())
        {
            XMLReader.readNext();
            break;
        }

        if (XMLReader.isStartElement())
        {
            skipUnknownElement();
        }
        else
        {
            XMLReader.readNext();
        }
    }
}

void GUIInfo::addProcess(Process* process)
{
    processes[process->processKey] = process;
}

void GUIInfo::addService(Service* service)
{
    services[service->serviceName] = service;
}

void GUIInfo::addCommand(Command* command)
{
    commands[command->commandName] = command;
}

void GUIInfo::clear()
{
    foreach(Process* process, processes)
    {
        process->servicesAsSender.clear();
        process->servicesAsReceiver.clear();
        process->commands.clear();
        delete process;
    }

    foreach(Service* service, services)
    {
        service->sender = NULL;
        service->receivers.clear();
        delete service;
    }

    foreach(Command* command, commands)
    {
        command->receivers.clear();
        delete command;
    }

    processes.clear();
    services.clear();
    commands.clear();
}

GUIInfo::~GUIInfo()
{
    clear();
}
