#ifndef TESTSERVICESUBSCRIBER_H
#define TESTSERVICESUBSCRIBER_H

#include <QObject>
#include "DIALOGapi.h"

class TESTServiceSubscriber : public DIALOGServiceSubscriber
{
    Q_OBJECT
public:
    explicit TESTServiceSubscriber(QString name);

public slots:
    virtual void dataUpdatedSlot(QByteArray dataInit) override;

signals:

};

#endif // TESTSERVICESUBSCRIBER_H
