#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <QMainWindow>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QStorageInfo>
#include <QInputDialog>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ftpsystemmodel.h"
#include "clientthread.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

namespace Ui {
class MainWindow;
}

class ClientThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void recvFillList();
    void recvClearList();
    void recvInfo(const QString &info);
    void recvSuccess();
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_comboBox_activated(const QString &arg1);
    void on_tableView_clicked(const QModelIndex &index);
    void on_pushButton_clicked();
    void on_tableView_2_doubleClicked(const QModelIndex &index);
    void on_pushButton_4_clicked();
    void on_tableView_2_clicked(const QModelIndex &index);
    void on_pushButton_2_clicked();
    void on_comboBox_2_activated(const QString &arg1);
    void on_actionAbout_triggered();
    void on_actionDelete_triggered();
    void on_actionRename_triggered();
    void on_actionAdd_triggered();
private:
    Ui::MainWindow *ui;
    QFileSystemModel *localModel;
    FtpSystemModel *ftpModel;
    ClientThread* clientThread;

    QVector<ItemInfo> itemList;
    QString prevPath;
    bool isConnected = false;
};
