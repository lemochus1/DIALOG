#ifndef COMMUNICATIONGUI_H
#define COMMUNICATIONGUI_H

#include <QMainWindow>
#include <QCloseEvent>
#include "guiinfo.h"
#include "showservicebutton.h"
#include "showreceiverbutton.h"
#include "showcommandbutton.h"

void end(qint32 sig);

namespace Ui {
class CommunicationGUI;
}

class CommunicationGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit CommunicationGUI(QWidget *parent = 0);
    ~CommunicationGUI();
    void clearConnectedProcessesTable();
    void closeEvent(QCloseEvent *e);

public slots:
    void updateConnectedProcessesTableSlot(GUIInfo* guiInfoInit, GUIInfo* guiInfoOld);
    void serviceOutputSlot(QString serviceName, QByteArray message);
    void commandOutputSlot(QString commandName, QByteArray message);
    void communicationControlServerNotFoundSlot();
    void sortConnectedProcessesTableSlot(int index);
    void requestServiceSlot(QString serviceName);
    void unSubscribeServiceSlot(QString serviceName);
    void sendCommandMessageSlot(QString commandName, QString message);
    void registerCommandSlot(QString commandName);
    void unRegisterCommandSlot(QString commandName);

signals:
    void requestServiceSignal(QString serviceName);
    void unSubscribeServiceSignal(QString serviceName);
    void sendCommandMessageSignal(QString commandName, QString message);
    void registerCommandSignal(QString commandName);
    void unRegisterCommandSignal(QString commandName);

private slots:
    void on_resetButton_clicked();
    void on_searchButton_clicked();

private:
    Ui::CommunicationGUI *ui;
    QMap<QString, ShowServiceButton*> showServiceButtons;
    QMap<QString, ShowReceiverButton*> showReceiverButtons;
    QMap<QString, ShowCommandButton*> showCommandButtons;
    GUIInfo* guiInfo;
    QMap<QString, int> subscribedServicesCount;
    QMap<QString, int> registeredCommandsCount;
    bool communicationControlServerNotFound;
};

#endif // COMMUNICATIONGUI_H
