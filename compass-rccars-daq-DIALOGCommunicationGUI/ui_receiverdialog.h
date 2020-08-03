/********************************************************************************
** Form generated from reading UI file 'receiverdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECEIVERDIALOG_H
#define UI_RECEIVERDIALOG_H

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
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ReceiverDialog
{
public:
    QPlainTextEdit *serviceOutput;
    QLabel *serviceOutputLabel;
    QLabel *serviceListLabel;
    QLabel *senderTableLabel;
    QTableWidget *senderTable;
    QListWidget *serviceList;
    QCheckBox *receiverAllServices;
    QPushButton *stopListeningButton;
    QPushButton *startListeningButton;

    void setupUi(QWidget *ReceiverDialog)
    {
        if (ReceiverDialog->objectName().isEmpty())
            ReceiverDialog->setObjectName(QStringLiteral("ReceiverDialog"));
        ReceiverDialog->resize(789, 436);
        serviceOutput = new QPlainTextEdit(ReceiverDialog);
        serviceOutput->setObjectName(QStringLiteral("serviceOutput"));
        serviceOutput->setGeometry(QRect(10, 230, 771, 161));
        serviceOutputLabel = new QLabel(ReceiverDialog);
        serviceOutputLabel->setObjectName(QStringLiteral("serviceOutputLabel"));
        serviceOutputLabel->setGeometry(QRect(10, 210, 63, 17));
        serviceListLabel = new QLabel(ReceiverDialog);
        serviceListLabel->setObjectName(QStringLiteral("serviceListLabel"));
        serviceListLabel->setGeometry(QRect(10, 10, 258, 17));
        senderTableLabel = new QLabel(ReceiverDialog);
        senderTableLabel->setObjectName(QStringLiteral("senderTableLabel"));
        senderTableLabel->setGeometry(QRect(280, 10, 63, 17));
        senderTable = new QTableWidget(ReceiverDialog);
        if (senderTable->columnCount() < 4)
            senderTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        senderTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        senderTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        senderTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        senderTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        senderTable->setObjectName(QStringLiteral("senderTable"));
        senderTable->setGeometry(QRect(280, 31, 501, 171));
        serviceList = new QListWidget(ReceiverDialog);
        serviceList->setObjectName(QStringLiteral("serviceList"));
        serviceList->setGeometry(QRect(10, 31, 256, 171));
        receiverAllServices = new QCheckBox(ReceiverDialog);
        receiverAllServices->setObjectName(QStringLiteral("receiverAllServices"));
        receiverAllServices->setGeometry(QRect(520, 210, 261, 22));
        stopListeningButton = new QPushButton(ReceiverDialog);
        stopListeningButton->setObjectName(QStringLiteral("stopListeningButton"));
        stopListeningButton->setGeometry(QRect(671, 401, 110, 27));
        startListeningButton = new QPushButton(ReceiverDialog);
        startListeningButton->setObjectName(QStringLiteral("startListeningButton"));
        startListeningButton->setGeometry(QRect(551, 401, 110, 27));

        retranslateUi(ReceiverDialog);

        QMetaObject::connectSlotsByName(ReceiverDialog);
    } // setupUi

    void retranslateUi(QWidget *ReceiverDialog)
    {
        ReceiverDialog->setWindowTitle(QApplication::translate("ReceiverDialog", "Form", 0));
        serviceOutputLabel->setText(QApplication::translate("ReceiverDialog", "Output:", 0));
        serviceListLabel->setText(QApplication::translate("ReceiverDialog", "Subscribed services:", 0));
        senderTableLabel->setText(QApplication::translate("ReceiverDialog", "Sender:", 0));
        QTableWidgetItem *___qtablewidgetitem = senderTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("ReceiverDialog", "Name of Process", 0));
        QTableWidgetItem *___qtablewidgetitem1 = senderTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("ReceiverDialog", "Address", 0));
        QTableWidgetItem *___qtablewidgetitem2 = senderTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("ReceiverDialog", "Port", 0));
        QTableWidgetItem *___qtablewidgetitem3 = senderTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("ReceiverDialog", "PID", 0));
        receiverAllServices->setText(QApplication::translate("ReceiverDialog", "See output of all subscribed services", 0));
        stopListeningButton->setText(QApplication::translate("ReceiverDialog", "Stop listening", 0));
        startListeningButton->setText(QApplication::translate("ReceiverDialog", "Start listening", 0));
    } // retranslateUi

};

namespace Ui {
    class ReceiverDialog: public Ui_ReceiverDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RECEIVERDIALOG_H
