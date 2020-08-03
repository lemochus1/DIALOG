/********************************************************************************
** Form generated from reading UI file 'servicedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVICEDIALOG_H
#define UI_SERVICEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ServiceDialog
{
public:
    QListWidget *serviceList;
    QTableWidget *receiverTable;
    QPlainTextEdit *serviceOutput;
    QLabel *receiverTableLabel;
    QLabel *serviceListLabel;
    QLabel *serviceOutputLabel;
    QPushButton *startListeningButton;
    QPushButton *stopListeningButton;

    void setupUi(QWidget *ServiceDialog)
    {
        if (ServiceDialog->objectName().isEmpty())
            ServiceDialog->setObjectName(QStringLiteral("ServiceDialog"));
        ServiceDialog->resize(789, 436);
        serviceList = new QListWidget(ServiceDialog);
        serviceList->setObjectName(QStringLiteral("serviceList"));
        serviceList->setGeometry(QRect(10, 31, 256, 171));
        receiverTable = new QTableWidget(ServiceDialog);
        if (receiverTable->columnCount() < 4)
            receiverTable->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        receiverTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        receiverTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        receiverTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        receiverTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        receiverTable->setObjectName(QStringLiteral("receiverTable"));
        receiverTable->setGeometry(QRect(280, 31, 501, 171));
        serviceOutput = new QPlainTextEdit(ServiceDialog);
        serviceOutput->setObjectName(QStringLiteral("serviceOutput"));
        serviceOutput->setGeometry(QRect(10, 230, 771, 161));
        receiverTableLabel = new QLabel(ServiceDialog);
        receiverTableLabel->setObjectName(QStringLiteral("receiverTableLabel"));
        receiverTableLabel->setGeometry(QRect(280, 10, 63, 17));
        serviceListLabel = new QLabel(ServiceDialog);
        serviceListLabel->setObjectName(QStringLiteral("serviceListLabel"));
        serviceListLabel->setGeometry(QRect(10, 10, 258, 17));
        serviceOutputLabel = new QLabel(ServiceDialog);
        serviceOutputLabel->setObjectName(QStringLiteral("serviceOutputLabel"));
        serviceOutputLabel->setGeometry(QRect(10, 210, 63, 17));
        startListeningButton = new QPushButton(ServiceDialog);
        startListeningButton->setObjectName(QStringLiteral("startListeningButton"));
        startListeningButton->setGeometry(QRect(551, 401, 110, 27));
        stopListeningButton = new QPushButton(ServiceDialog);
        stopListeningButton->setObjectName(QStringLiteral("stopListeningButton"));
        stopListeningButton->setGeometry(QRect(671, 401, 110, 27));

        retranslateUi(ServiceDialog);

        QMetaObject::connectSlotsByName(ServiceDialog);
    } // setupUi

    void retranslateUi(QWidget *ServiceDialog)
    {
        ServiceDialog->setWindowTitle(QApplication::translate("ServiceDialog", "Form", 0));
        QTableWidgetItem *___qtablewidgetitem = receiverTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("ServiceDialog", "Name of Process", 0));
        QTableWidgetItem *___qtablewidgetitem1 = receiverTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("ServiceDialog", "Address", 0));
        QTableWidgetItem *___qtablewidgetitem2 = receiverTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("ServiceDialog", "Port", 0));
        QTableWidgetItem *___qtablewidgetitem3 = receiverTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("ServiceDialog", "PID", 0));
        receiverTableLabel->setText(QApplication::translate("ServiceDialog", "Receivers:", 0));
        serviceListLabel->setText(QApplication::translate("ServiceDialog", "Provided services:", 0));
        serviceOutputLabel->setText(QApplication::translate("ServiceDialog", "Output:", 0));
        startListeningButton->setText(QApplication::translate("ServiceDialog", "Start listening", 0));
        stopListeningButton->setText(QApplication::translate("ServiceDialog", "Stop listening", 0));
    } // retranslateUi

};

namespace Ui {
    class ServiceDialog: public Ui_ServiceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVICEDIALOG_H
