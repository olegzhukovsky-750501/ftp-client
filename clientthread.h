#pragma once

#include "ftpclient.h"

enum Task{Connect, Disconnect, ChangeDir, Download, Upload, RemoveFile, RemoveDir, Rename, MakeDir};

class ClientThread : public QThread
{
    friend class MainWindow;
    Q_OBJECT
public:
    explicit ClientThread();
    ~ClientThread();

    Task task;

    QStringList arglist;
protected:
    void run();
private:
    FtpClient *client;
    void fillList();
private slots:
    void stop();
signals:
    void emitFillList();
    void emitSuccess();
    void emitClearList();
};
