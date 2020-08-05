#include <QCoreApplication>
#include <signal.h>

#include <iostream>
#include <QtDebug>
#include <QtGlobal>
#include "daqdebugger.h"

#include "DIALOGapi.h"

#include "testprocesscontroller.h"

#include <stdio.h>
#include <stdlib.h>

DIALOGProcess* process;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    process->stopSlot();
}

QString PROCESS_NAME = "Unknown";
QString LOG_FILE_PATH = "default.log";

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = msg;
        break;
    case QtWarningMsg:
        txt = QString("WARNING: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("CRITICAL: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("FATAL: %1").arg(msg);
        abort();
    default:
        break;
    }

    QFile outFile(LOG_FILE_PATH);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << PROCESS_NAME << ": " << txt << endl;
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    //DAQDebugger::init(argv[0]);
    qInstallMessageHandler(myMessageHandler);

    if (app.arguments().size() < 2) {
        qDebug() << "Config file was not defined in the process arguments.";
        return 1;
    }

    QString configPath = app.arguments()[1];

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open  " << configPath <<" to load process configuration.";
        return 2;
    }

    QTextStream in(&configFile);
    QString configuration = in.readAll();
    in.flush();
    configFile.close();

    for (int i = 2; i < app.arguments().size(); i++){
        QStringList beforeAndAfter = app.arguments()[i].split('|');
        if (beforeAndAfter.size() == 2) {
            configuration.replace(beforeAndAfter.first(), beforeAndAfter.last());
        } else if (i == 2) {
            LOG_FILE_PATH = app.arguments()[i];
        } else {
            qDebug() << "Invalid argument forwarded: " << app.arguments()[i];
        }
    }

    TESTProcessController controller;
    controller.setupProcess(configuration);

    if (!LOG_FILE_PATH.isEmpty()) {
        PROCESS_NAME = controller.getProcessName();
        qInstallMessageHandler(myMessageHandler);
    }
    controller.startProcess();
    controller.startSenders();

    std::cout<< "After run";
    (void) signal(SIGINT, end);
    return app.exec();
}
