#pragma once

#include <iostream>
#include <fstream>
#include <QMessageBox>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ftpsystemmodel.h"
#include "infothread.h"

const int BUFLEN = 512;
const int DATABUFLEN = 4096;

class FtpClient
{
friend class ClientThread;
friend class MainWindow;
public:
    FtpClient();
    ~FtpClient();

    int waitForSocket(SOCKET s, bool read, bool write);
    int init_csock();
    int init_dsock();
    int establishConnect();
    QString read_serv(long tv_sec, long tv_usec);
    int get(QString fileName, QString localPath);
    int getFileSize(QString fileName);
    int login();
    int cd(QString dir);
    int deleteFile(QString fileName);
    int deleteDir(QString dirName);
    int mkDir(QString name);
    int list();
    int rename(QString src, QString dst);
    char* getWorkingDir();
    static bool typeLessThan(const ItemInfo &i1, const ItemInfo &i2);
    int sendFile(QString filePath, QString fileName);
    int getStateCode(char *response);
    int disconnect();
    void setConnectData(QString ip_addr, QString username, QString password, QString port);
    void forceDisconnect();
private:
    void execCmd(QString cmd);

    QString ip_addr;
    QString username;
    QString password;
    QString port;

    WSADATA wlib;
    SOCKET cs;
    SOCKET ds;
    InfoThread* infoThread;

    vector<char*> shortNameItemList;
    vector<char*> fullNameItemList;
    QStringList fileSizeList;
    QVector<ItemInfo> itemList;

    QString workingDir;
};
