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

    process->start(QThread::TimeCriticalPriority);

    for (int i = 0; i<20; i++) {
        process->sendCommandSlot("Command", QByteArray("Nothing"));
        QThread::sleep(1);
        process->sendDirectCommandSlot("Command", QByteArray("Direct"), "CommandHandler");
        QThread::sleep(1);
        process->sendDirectCommandSlot("Command", QByteArray("Url"), "desktop-jttvct0", 35461);
        QThread::sleep(1);
    }

    std::cout<< "After run";
    (void) signal(SIGINT, end);
    return app->exec();
}
