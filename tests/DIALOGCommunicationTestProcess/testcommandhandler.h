#ifndef TESTCOMMANDHANDLER_H
#define TESTCOMMANDHANDLER_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTCommandHandler : public DIALOGCommandHandler
{
    Q_OBJECT
public:
    explicit TESTCommandHandler(const QString& name);

private slots:
    virtual void commandReceivedSlot(const QByteArray& message) override;
    virtual void controlServerConnectedSlot() override;
    virtual void controlServerUnavailableErrorSlot() override;
};

#endif // TESTCOMMANDHANDLER_H
