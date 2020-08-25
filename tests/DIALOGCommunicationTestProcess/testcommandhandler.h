#ifndef TESTCOMMANDHANDLER_H
#define TESTCOMMANDHANDLER_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTCommandHandler : public DIALOGCommandHandler
{
    Q_OBJECT
public:
    explicit TESTCommandHandler(QString name);

public slots:
    virtual void commandReceivedSlot(QByteArray message) override;

signals:

};

#endif // TESTCOMMANDHANDLER_H
