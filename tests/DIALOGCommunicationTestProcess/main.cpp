#include <QCoreApplication>
#include <signal.h>

#include <iostream>
#include <QtDebug>
#include <QtGlobal>
#include "daqdebugger.h"

#include "dialogapi.h"

#include "testprocesscontroller.h"
#include "apimessagelogger.h"

#include <stdio.h>
#include <stdlib.h>

DIALOGProcess* process;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    process->stop();
}

QString LOG_FILE_PATH = "";

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString timestamp = "\"" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\"";

    QString txt;
    switch (type) {
    case QtWarningMsg:
        txt = timestamp + QString("  WARNING: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = timestamp + QString("  CRITICAL: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = timestamp + QString("  FATAL: %1").arg(msg);
        break;
    default:
        txt = msg;
        break;
    }

    fprintf(stderr, "%s\n", txt.toLocal8Bit().constData());
    if (!LOG_FILE_PATH.isEmpty()){
        QFile outFile(LOG_FILE_PATH);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
    }
    if (type == QtFatalMsg){
        abort();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

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

    if (!LOG_FILE_PATH.isEmpty()) {
        APIMessageLogger::GetInstance().setLogFile(LOG_FILE_PATH);
        qInstallMessageHandler(&myMessageHandler);
    }

    TESTProcessController controller;
    controller.setupProcess(configuration);
    controller.startProcess();
    controller.startSenders();

    (void) signal(SIGINT, end);
    return app.exec();
}
