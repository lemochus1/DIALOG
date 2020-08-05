#include "testcommandhandler.h"

TESTCommandHandler::TESTCommandHandler(QString name)
    : DIALOGCommand(name)
{

}

void TESTCommandHandler::commandReceivedSlot(QByteArray message)
{
    qDebug() << "Recieved:" << QString(message);
}
