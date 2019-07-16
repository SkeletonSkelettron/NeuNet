/********************************************************************************
** Form generated from reading UI file 'NeuNet.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NEUNET_H
#define UI_NEUNET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NeuNetClass
{
public:
    QAction *actionSave;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_2;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QTextEdit *textEdit;
    QWidget *tab_2;
    QMenuBar *menuBar;
    QMenu *menuMenu;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *NeuNetClass)
    {
        if (NeuNetClass->objectName().isEmpty())
            NeuNetClass->setObjectName(QString::fromUtf8("NeuNetClass"));
        NeuNetClass->resize(1090, 749);
        actionSave = new QAction(NeuNetClass);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        centralWidget = new QWidget(NeuNetClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        verticalLayout->setContentsMargins(0, 0, -1, -1);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(10);
        tabWidget->setFont(font);
        tabWidget->setLayoutDirection(Qt::LeftToRight);
        tabWidget->setAutoFillBackground(true);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        gridLayout_2 = new QGridLayout(tab);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        pushButton = new QPushButton(tab);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        gridLayout_2->addWidget(pushButton, 1, 1, 1, 1);

        pushButton_2 = new QPushButton(tab);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        gridLayout_2->addWidget(pushButton_2, 1, 0, 1, 1);

        textEdit = new QTextEdit(tab);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setEnabled(true);
        QFont font1;
        font1.setFamily(QString::fromUtf8("Source Code Pro"));
        font1.setStyleStrategy(QFont::PreferAntialias);
        textEdit->setFont(font1);

        gridLayout_2->addWidget(textEdit, 0, 0, 1, 2);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        tabWidget->addTab(tab_2, QString());

        verticalLayout->addWidget(tabWidget);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        NeuNetClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(NeuNetClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1090, 21));
        menuMenu = new QMenu(menuBar);
        menuMenu->setObjectName(QString::fromUtf8("menuMenu"));
        NeuNetClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(NeuNetClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        NeuNetClass->setStatusBar(statusBar);

        menuBar->addAction(menuMenu->menuAction());
        menuMenu->addAction(actionSave);

        retranslateUi(NeuNetClass);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(NeuNetClass);
    } // setupUi

    void retranslateUi(QMainWindow *NeuNetClass)
    {
        NeuNetClass->setWindowTitle(QApplication::translate("NeuNetClass", "NeuNet", nullptr));
        actionSave->setText(QApplication::translate("NeuNetClass", "Save", nullptr));
        pushButton->setText(QApplication::translate("NeuNetClass", "PushButton", nullptr));
        pushButton_2->setText(QApplication::translate("NeuNetClass", "Test", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("NeuNetClass", "NetSettings", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("NeuNetClass", "NetTest", nullptr));
        menuMenu->setTitle(QApplication::translate("NeuNetClass", "Menu", nullptr));
    } // retranslateUi

};

namespace Ui {
    class NeuNetClass: public Ui_NeuNetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEUNET_H
