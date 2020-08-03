#ifndef COMMANDDIALOG_H
#define COMMANDDIALOG_H

#include <QWidget>
#include "server.h"

namespace Ui {
class CommandDialog;
}

class CommandDialog : public QWidget
{
    Q_OBJECT

public:
    explicit CommandDialog(Process* processInit, QWidget *parent = 0);
    ~CommandDialog();
    void updateCommandDialog(Process* processInit);
    void updateCommandOutput(QString commandName, QByteArray message);
    void commandsAvailable();
    void noCommandAvailable();
    void hideEvent(QHideEvent * event);
    Process* process;
    Command* selectedCommand;
    QString selectedCommandName;
    bool isCommandRegistered;
    QList<QString> listOfRegisteredCommands;

public slots:
    void selectedCommandChangedSlot();
    void sendCommandMessageSlot();
    void receiverAllCommandsSlot();
    void startListeningSlot();
    void stopListeningSlot();

signals:
    void sendCommandMessageSignal(QString commandName, QString message);
    void registerCommandSignal(QString commandName);
    void unRegisterCommandSignal(QString commandName);

private:
    Ui::CommandDialog *ui;
};

#endif // COMMANDDIALOG_H
