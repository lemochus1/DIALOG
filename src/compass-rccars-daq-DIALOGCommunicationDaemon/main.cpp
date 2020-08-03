#include <QCoreApplication>
#include <signal.h>

#include "daemonthread.h"
#include "server.h"
#include "receiverprocessorthread.h"
#include "senderprocessorthread.h"
#include "daqdebugger.h"
#include <iostream>
QCoreApplication* app;
DaemonThread* daemonThread;

ReceiverProcessorThread* receiverProcessorThread;
SenderProcessorThread* senderProcessorThread;
Server* server;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    daemonThread->stop();
}

int main(int argc, char *argv[])
{
    app = new QCoreApplication(argc, argv);

    DAQDebugger::init(argv[0]);

    daemonThread = new DaemonThread();

    receiverProcessorThread = new ReceiverProcessorThread();
    senderProcessorThread = new SenderProcessorThread();

    server = new Server("DIALOGCommunicationDaemon", Monitoring, getenv("DIALOG_CONTROL_SERVER_ADDRESS"), QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt(), senderProcessorThread, receiverProcessorThread);

    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::updateDIALOGInfoSignal, daemonThread, &DaemonThread::updateDIALOGInfoSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::serviceOutputSignal, daemonThread, &DaemonThread::serviceOutputSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::commandOutputSignal, daemonThread, &DaemonThread::commandOutputSlot);
    QObject::connect(daemonThread, &DaemonThread::requestServiceSignal, senderProcessorThread, &SenderProcessorThread::requestServiceSlot);
    QObject::connect(daemonThread, &DaemonThread::registerCommandSignal, senderProcessorThread, &SenderProcessorThread::registerCommandSlot);

    QObject::connect(daemonThread, &DaemonThread::finished, daemonThread, &QObject::deleteLater);
    QObject::connect(daemonThread, &DaemonThread::destroyed, server, &Server::stopSlot);
    QObject::connect(server, &Server::destroyed, app, &QCoreApplication::quit);

    daemonThread->start();

    server->start(QThread::TimeCriticalPriority);

    (void) signal(SIGINT, end); // forwarding of standart signal to end function
    return app->exec();
}
