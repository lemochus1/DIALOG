#include <QCoreApplication>

#include "server.h"

#include <signal.h>
#include "daqdebugger.h"

QString LOG_FILE_PATH = "";

Server* server;

void end(qint32 sig)
{
    std::cout << "You killed me with " << sig << std::endl;
    server->stop();
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
    case QtWarningMsg:
        txt = QString("WARNING: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("CRITICAL: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("FATAL: %1").arg(msg);
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

    if (app.arguments().size() == 2) {
        LOG_FILE_PATH = app.arguments()[1];
        qInstallMessageHandler(&myMessageHandler);
    }

    server = new Server("ControlServer", ControlServer, "127.0.0.1", 8081);
    QObject::connect(server, &Server::destroyed, &app, &QCoreApplication::quit);
    server->start(QThread::TimeCriticalPriority);

    (void) signal(SIGINT, end);
    return app.exec();
}
