#include <QCoreApplication>

#include "server.h"

#include <signal.h>
#include "daqdebugger.h"

QCoreApplication* app;
QThread* serverThread;
Server* server;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    server->stop();
}

int main(int argc, char *argv[])
{
    DAQDebugger::init(argv[0]);

    app = new QCoreApplication(argc, argv);

    //server = new Server("ControlServer", ControlServer, getenv("DIALOG_CONTROL_SERVER_ADDRESS"), QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt());
    server = new Server("ControlServer", ControlServer, "127.0.0.1", 8081);


    QObject::connect(server, &Server::destroyed, app, &QCoreApplication::quit);
    server->start(QThread::TimeCriticalPriority);

    (void) signal(SIGINT, end);

    return app->exec();
}
