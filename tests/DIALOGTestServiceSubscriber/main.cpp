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

    process = new DIALOGProcess("CommandSender");

    QObject::connect(process, &DIALOGProcess::serverDestroyed, app, &QCoreApplication::quit);


    DIALOGServiceSubscriber* subscriber = process->requestService("Service");

    QObject::connect(subscriber, &DIALOGServiceSubscriber::dataUpdatedSignal,
                [](QByteArray message){std::cout<< message.toStdString()<<std::endl;});


    process->start(QThread::TimeCriticalPriority);

    std::cout<< "After run";
    (void) signal(SIGINT, end);
    return app->exec();
}
