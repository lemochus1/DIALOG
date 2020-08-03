#include "communicationgui.h"
#include <QApplication>

#include "server.h"
#include "receiverprocessorthread.h"
#include "senderprocessorthread.h"
#include <signal.h>
#include "daqdebugger.h"

QCoreApplication* app;
ReceiverProcessorThread* receiverProcessorThread;
SenderProcessorThread* senderProcessorThread;
Server* server;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    server->stop();
}

int main(int argc, char *argv[])
{
    app = new QApplication(argc, argv);

    DAQDebugger::init(argv[0]);

    CommunicationGUI w;

    senderProcessorThread = new SenderProcessorThread();
    receiverProcessorThread = new ReceiverProcessorThread();
    server = new Server("GUI", Monitoring, getenv("DIALOG_CONTROL_SERVER_ADDRESS"), QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt(), senderProcessorThread, receiverProcessorThread);

    QObject::connect(&w, &CommunicationGUI::requestServiceSignal, senderProcessorThread, &SenderProcessorThread::requestServiceSlot);
    QObject::connect(&w, &CommunicationGUI::unSubscribeServiceSignal, senderProcessorThread, &SenderProcessorThread::unSubscribeServiceSlot);
    QObject::connect(&w, &CommunicationGUI::sendCommandMessageSignal, senderProcessorThread, &SenderProcessorThread::sendCommandMessageSlot);
    QObject::connect(&w, &CommunicationGUI::registerCommandSignal, senderProcessorThread, &SenderProcessorThread::registerCommandSlot);
    QObject::connect(&w, &CommunicationGUI::unRegisterCommandSignal, senderProcessorThread, &SenderProcessorThread::unRegisterCommandSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::updateConnectedProcessesTableSignal, &w, &CommunicationGUI::updateConnectedProcessesTableSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::serviceOutputSignal, &w, &CommunicationGUI::serviceOutputSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::commandOutputSignal, &w, &CommunicationGUI::commandOutputSlot);
    QObject::connect(receiverProcessorThread, &ReceiverProcessorThread::communicationControlServerNotFoundSignal, &w, &CommunicationGUI::communicationControlServerNotFoundSlot);
    QObject::connect(server, &Server::destroyed, app, &QCoreApplication::quit);

    server->start(QThread::TimeCriticalPriority);

    (void) signal(SIGINT, end);

    w.show();

    return app->exec();
}
