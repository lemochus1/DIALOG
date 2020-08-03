/********************************************************************************
** Form generated from reading UI file 'commanddialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMANDDIALOG_H
#define UI_COMMANDDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CommandDialog
{
public:
    QListWidget *commandList;
    QLabel *commandListLabel;
    QLabel *commandMessageLabel;
    QPlainTextEdit *commandMessage;
    QPushButton *sendCommandButton;
    QPlainTextEdit *commandOutput;
    QCheckBox *receiverAllCommands;
    QLabel *commandOutputLabel;
    QPushButton *stopListeningButton;
    QPushButton *startListeningButton;

    void setupUi(QWidget *CommandDialog)
    {
        if (CommandDialog->objectName().isEmpty())
            CommandDialog->setObjectName(QStringLiteral("CommandDialog"));
        CommandDialog->resize(789, 436);
        commandList = new QListWidget(CommandDialog);
        commandList->setObjectName(QStringLiteral("commandList"));
        commandList->setGeometry(QRect(10, 31, 256, 171));
        commandListLabel = new QLabel(CommandDialog);
        commandListLabel->setObjectName(QStringLiteral("commandListLabel"));
        commandListLabel->setGeometry(QRect(10, 10, 258, 17));
        commandMessageLabel = new QLabel(CommandDialog);
        commandMessageLabel->setObjectName(QStringLiteral("commandMessageLabel"));
        commandMessageLabel->setGeometry(QRect(280, 10, 150, 17));
        commandMessage = new QPlainTextEdit(CommandDialog);
        commandMessage->setObjectName(QStringLiteral("commandMessage"));
        commandMessage->setGeometry(QRect(280, 31, 501, 134));
        sendCommandButton = new QPushButton(CommandDialog);
        sendCommandButton->setObjectName(QStringLiteral("sendCommandButton"));
        sendCommandButton->setGeometry(QRect(660, 175, 121, 27));
        commandOutput = new QPlainTextEdit(CommandDialog);
        commandOutput->setObjectName(QStringLiteral("commandOutput"));
        commandOutput->setGeometry(QRect(10, 230, 771, 161));
        receiverAllCommands = new QCheckBox(CommandDialog);
        receiverAllCommands->setObjectName(QStringLiteral("receiverAllCommands"));
        receiverAllCommands->setGeometry(QRect(493, 210, 288, 22));
        commandOutputLabel = new QLabel(CommandDialog);
        commandOutputLabel->setObjectName(QStringLiteral("commandOutputLabel"));
        commandOutputLabel->setGeometry(QRect(10, 210, 400, 17));
        stopListeningButton = new QPushButton(CommandDialog);
        stopListeningButton->setObjectName(QStringLiteral("stopListeningButton"));
        stopListeningButton->setGeometry(QRect(671, 401, 110, 27));
        startListeningButton = new QPushButton(CommandDialog);
        startListeningButton->setObjectName(QStringLiteral("startListeningButton"));
        startListeningButton->setGeometry(QRect(551, 401, 110, 27));

        retranslateUi(CommandDialog);

        QMetaObject::connectSlotsByName(CommandDialog);
    } // setupUi

    void retranslateUi(QWidget *CommandDialog)
    {
        CommandDialog->setWindowTitle(QApplication::translate("CommandDialog", "Form", 0));
        commandListLabel->setText(QApplication::translate("CommandDialog", "Commands:", 0));
        commandMessageLabel->setText(QApplication::translate("CommandDialog", "Command message:", 0));
        sendCommandButton->setText(QApplication::translate("CommandDialog", "Send command", 0));
        receiverAllCommands->setText(QApplication::translate("CommandDialog", "See received messages of all commands", 0));
        commandOutputLabel->setText(QApplication::translate("CommandDialog", "Received messages:", 0));
        stopListeningButton->setText(QApplication::translate("CommandDialog", "Stop listening", 0));
        startListeningButton->setText(QApplication::translate("CommandDialog", "Start listening", 0));
    } // retranslateUi

};

namespace Ui {
    class CommandDialog: public Ui_CommandDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMANDDIALOG_H
