/********************************************************************************
** Form generated from reading UI file 'communicationgui.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMUNICATIONGUI_H
#define UI_COMMUNICATIONGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CommunicationGUI
{
public:
    QWidget *centralWidget;
    QTableWidget *connectedProcessesTable;
    QLabel *nameOfProcessLabel;
    QLineEdit *nameOfProcessText;
    QLineEdit *nameOfCommandText;
    QLabel *nameOfCommandLabel;
    QLineEdit *nameOfProvidedServiceText;
    QLabel *nameOfProvidedServiceLabel;
    QLineEdit *nameOfSubscribedServiceText;
    QLabel *nameOfSubscribedServiceLabel;
    QPushButton *searchButton;
    QPushButton *resetButton;
    QLabel *pidLabel;
    QLineEdit *pidText;
    QLabel *addressLabel;
    QLineEdit *addressText;
    QLineEdit *portText;
    QLabel *portLabel;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CommunicationGUI)
    {
        if (CommunicationGUI->objectName().isEmpty())
            CommunicationGUI->setObjectName(QStringLiteral("CommunicationGUI"));
        CommunicationGUI->resize(1210, 490);
        centralWidget = new QWidget(CommunicationGUI);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        connectedProcessesTable = new QTableWidget(centralWidget);
        if (connectedProcessesTable->columnCount() < 7)
            connectedProcessesTable->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        connectedProcessesTable->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        connectedProcessesTable->setObjectName(QStringLiteral("connectedProcessesTable"));
        connectedProcessesTable->setGeometry(QRect(0, 80, 751, 271));
        connectedProcessesTable->setMaximumSize(QSize(751, 16777215));
        connectedProcessesTable->setColumnCount(7);
        nameOfProcessLabel = new QLabel(centralWidget);
        nameOfProcessLabel->setObjectName(QStringLiteral("nameOfProcessLabel"));
        nameOfProcessLabel->setGeometry(QRect(10, 10, 121, 17));
        nameOfProcessText = new QLineEdit(centralWidget);
        nameOfProcessText->setObjectName(QStringLiteral("nameOfProcessText"));
        nameOfProcessText->setGeometry(QRect(130, 5, 113, 27));
        nameOfCommandText = new QLineEdit(centralWidget);
        nameOfCommandText->setObjectName(QStringLiteral("nameOfCommandText"));
        nameOfCommandText->setGeometry(QRect(919, 5, 113, 27));
        nameOfCommandLabel = new QLabel(centralWidget);
        nameOfCommandLabel->setObjectName(QStringLiteral("nameOfCommandLabel"));
        nameOfCommandLabel->setGeometry(QRect(799, 10, 121, 17));
        nameOfProvidedServiceText = new QLineEdit(centralWidget);
        nameOfProvidedServiceText->setObjectName(QStringLiteral("nameOfProvidedServiceText"));
        nameOfProvidedServiceText->setGeometry(QRect(661, 5, 113, 27));
        nameOfProvidedServiceLabel = new QLabel(centralWidget);
        nameOfProvidedServiceLabel->setObjectName(QStringLiteral("nameOfProvidedServiceLabel"));
        nameOfProvidedServiceLabel->setGeometry(QRect(529, 10, 131, 17));
        nameOfSubscribedServiceText = new QLineEdit(centralWidget);
        nameOfSubscribedServiceText->setObjectName(QStringLiteral("nameOfSubscribedServiceText"));
        nameOfSubscribedServiceText->setGeometry(QRect(661, 35, 113, 27));
        nameOfSubscribedServiceLabel = new QLabel(centralWidget);
        nameOfSubscribedServiceLabel->setObjectName(QStringLiteral("nameOfSubscribedServiceLabel"));
        nameOfSubscribedServiceLabel->setGeometry(QRect(529, 40, 131, 17));
        searchButton = new QPushButton(centralWidget);
        searchButton->setObjectName(QStringLiteral("searchButton"));
        searchButton->setGeometry(QRect(1052, 5, 91, 27));
        resetButton = new QPushButton(centralWidget);
        resetButton->setObjectName(QStringLiteral("resetButton"));
        resetButton->setGeometry(QRect(1052, 35, 91, 27));
        pidLabel = new QLabel(centralWidget);
        pidLabel->setObjectName(QStringLiteral("pidLabel"));
        pidLabel->setGeometry(QRect(10, 40, 121, 17));
        pidText = new QLineEdit(centralWidget);
        pidText->setObjectName(QStringLiteral("pidText"));
        pidText->setGeometry(QRect(130, 35, 113, 27));
        addressLabel = new QLabel(centralWidget);
        addressLabel->setObjectName(QStringLiteral("addressLabel"));
        addressLabel->setGeometry(QRect(260, 10, 121, 17));
        addressText = new QLineEdit(centralWidget);
        addressText->setObjectName(QStringLiteral("addressText"));
        addressText->setGeometry(QRect(380, 5, 113, 27));
        portText = new QLineEdit(centralWidget);
        portText->setObjectName(QStringLiteral("portText"));
        portText->setGeometry(QRect(380, 35, 113, 27));
        portLabel = new QLabel(centralWidget);
        portLabel->setObjectName(QStringLiteral("portLabel"));
        portLabel->setGeometry(QRect(260, 40, 121, 17));
        CommunicationGUI->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CommunicationGUI);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1210, 25));
        CommunicationGUI->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CommunicationGUI);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        CommunicationGUI->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CommunicationGUI);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        CommunicationGUI->setStatusBar(statusBar);

        retranslateUi(CommunicationGUI);

        QMetaObject::connectSlotsByName(CommunicationGUI);
    } // setupUi

    void retranslateUi(QMainWindow *CommunicationGUI)
    {
        CommunicationGUI->setWindowTitle(QApplication::translate("CommunicationGUI", "DIALOG CommunicationGUI", 0));
        QTableWidgetItem *___qtablewidgetitem = connectedProcessesTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("CommunicationGUI", "Name of Process", 0));
        QTableWidgetItem *___qtablewidgetitem1 = connectedProcessesTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("CommunicationGUI", "Address", 0));
        QTableWidgetItem *___qtablewidgetitem2 = connectedProcessesTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("CommunicationGUI", "Port", 0));
        QTableWidgetItem *___qtablewidgetitem3 = connectedProcessesTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("CommunicationGUI", "PID", 0));
        QTableWidgetItem *___qtablewidgetitem4 = connectedProcessesTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("CommunicationGUI", "Provided services", 0));
        QTableWidgetItem *___qtablewidgetitem5 = connectedProcessesTable->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("CommunicationGUI", "Subscribed services", 0));
        QTableWidgetItem *___qtablewidgetitem6 = connectedProcessesTable->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QApplication::translate("CommunicationGUI", "Commands", 0));
        nameOfProcessLabel->setText(QApplication::translate("CommunicationGUI", "Name of Process:", 0));
        nameOfCommandLabel->setText(QApplication::translate("CommunicationGUI", "Command:", 0));
        nameOfProvidedServiceLabel->setText(QApplication::translate("CommunicationGUI", "Provided service:", 0));
        nameOfSubscribedServiceLabel->setText(QApplication::translate("CommunicationGUI", "Subscribed service:", 0));
        searchButton->setText(QApplication::translate("CommunicationGUI", "Search", 0));
        resetButton->setText(QApplication::translate("CommunicationGUI", "Reset", 0));
        pidLabel->setText(QApplication::translate("CommunicationGUI", "PID:", 0));
        addressLabel->setText(QApplication::translate("CommunicationGUI", "Address:", 0));
        portLabel->setText(QApplication::translate("CommunicationGUI", "Port:", 0));
    } // retranslateUi

};

namespace Ui {
    class CommunicationGUI: public Ui_CommunicationGUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMUNICATIONGUI_H
