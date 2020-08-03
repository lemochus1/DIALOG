#include "dialogInfo.h"

DIALOGInfo::DIALOGInfo(QObject *parent) :
    QObject(parent)
{
}

void DIALOGInfo::readDIALOGInfo(QString info)
{
    XMLReader.clear();
    XMLReader.addData(info);
    while(!XMLReader.atEnd())
    {
        if(XMLReader.isStartElement())
        {
            if(XMLReader.name() == "processes")
            {
                readDIALOGInfoProcessElement();
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

void DIALOGInfo::readDIALOGInfoProcessElement()
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
                readDIALOGInfoServiceOrCommandElement(processes[processKey]);
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

void DIALOGInfo::readDIALOGInfoServiceOrCommandElement(Process* process)
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
                readDIALOGInfoReceiverElement(service);
            }
            else if (XMLReader.name() == "command")
            {
                QXmlStreamAttributes atr = XMLReader.attributes();
                QString commandName = atr.value("name").toString();
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

void DIALOGInfo::readDIALOGInfoReceiverElement(Service* service)
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

void DIALOGInfo::skipUnknownElement()
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

void DIALOGInfo::addProcess(Process* process)
{
    processes[process->processKey] = process;
}

void DIALOGInfo::addService(Service* service)
{
    services[service->serviceName] = service;
}

void DIALOGInfo::addCommand(Command* command)
{
    commands[command->commandName] = command;
}

void DIALOGInfo::clear()
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

DIALOGInfo::~DIALOGInfo()
{
    clear();
}
