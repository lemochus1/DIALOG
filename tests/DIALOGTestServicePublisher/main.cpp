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

    process = new DIALOGProcess("ServicePublisher");

    QObject::connect(process, &DIALOGProcess::serverDestroyed, app, &QCoreApplication::quit);

    DIALOGServicePublisher* publisher = process->registerService("Service");

    process->start(QThread::TimeCriticalPriority);

    for (int i = 0; i<20; i++) {
        publisher->updateDataSlot(QByteArray("Nothing"));
        QThread::sleep(10);
    }

    std::cout<< "After run";
    (void) signal(SIGINT, end);
    return app->exec();
}
