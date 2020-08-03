#include <QCoreApplication>
#include <signal.h>

#include <iostream>
#include "daqdebugger.h"

#include "DIALOGapi.h"

QCoreApplication* app;
DIALOGProcess* process;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    process->stopSlot();
}

int main(int argc, char *argv[])
{
    DAQDebugger::init(argv[0]);
    app = new QCoreApplication(argc, argv);

    process = new DIALOGProcess("CommandHandler");
    process->setControlServerAdress("127.0.0.1", 8081);

    QObject::connect(process, &DIALOGProcess::serverDestroyed, app, &QCoreApplication::quit);

    DIALOGCommand* handler = process->registerCommand("Command");

    QObject::connect(handler, &DIALOGCommand::commandReceivedSignal,
                [](QByteArray message){std::cout<< message.toStdString()<<std::endl;});

    process->start(QThread::TimeCriticalPriority);

    std::cout<< "After run";
    (void) signal(SIGINT, end);
    return app->exec();
}
