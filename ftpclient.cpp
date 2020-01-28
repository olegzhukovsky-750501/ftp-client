#include "ftpclient.h"

using namespace std;

FtpClient::FtpClient()
{
    infoThread = new InfoThread();
}

int FtpClient::init_csock()
{
    int ret_code = WSAStartup(MAKEWORD(2, 2), &wlib);
    if(ret_code != 0)
    {
        infoThread->sendInfo("Initiating use of the Winsock DLL by a process failed\n"
                             "Error code: " + QString::number(ret_code) + "\n");
        return -1;
    }
    sockaddr_in address;

    cs = socket(AF_INET, SOCK_STREAM, 0);
    if(cs == INVALID_SOCKET)
    {
        int err_code = WSAGetLastError();
        infoThread->sendInfo("Creating control socket failed\n"
                             "Error code: " + QString::number(err_code) + "\n");
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_addr.toLocal8Bit().data());
    address.sin_port = htons(port.toInt());

    int result = ::connect(cs, (SOCKADDR*)&address, sizeof(address));

    if(result == SOCKET_ERROR)
    {
        int err_code = WSAGetLastError();
        infoThread->sendInfo("Control socket connection failed\n"
                             "Error code: " + QString::number(err_code) + "\n");
        return -1;
    }

    return 0;
}

int FtpClient::init_dsock()
{
    execCmd("PASV");
    char buf[128];

    QString response = read_serv(0, 500000);

    if(response.isEmpty())
    {
        infoThread->sendInfo("Reading server timeout\n");
        return 1;
    }

    strcpy(buf, response.toLocal8Bit().data());

    int a, b;
    char *tmp_char;
    tmp_char = strtok(buf, "(");
    tmp_char = strtok(NULL, "(");
    tmp_char = strtok(tmp_char, ")");
    int c, d, e, f;
    sscanf(tmp_char, "%d, %d, %d, %d, %d, %d", &c, &d,&e, &f, &a, &b);

    sockaddr_in address;
    int port = a*256 + b;

    ds = socket(AF_INET, SOCK_STREAM, 0);
    if(ds == INVALID_SOCKET)
    {
        int err_code = WSAGetLastError();
        infoThread->sendInfo("Creating data socket failed\n"
                             "Error code: " + QString::number(err_code) + "\n");
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_addr.toLocal8Bit().data());
    address.sin_port = htons(port);

    int result = ::connect(ds, (sockaddr *)&address, sizeof(address));

    if(result == SOCKET_ERROR)
    {
        int err_code = WSAGetLastError();
        infoThread->sendInfo("Data socket connection failed\n"
                             "Error code: " + QString::number(err_code) + "\n");
        return -1;
    }

    return 0;
}

int FtpClient::establishConnect()
{
    if(init_csock())
    {
        return -1;
    }

    QString response = read_serv(0, 500000);

    if(response.isEmpty())
    {
        return -2;
    }

    if(login())
    {
        return -3;
    }

    if(init_dsock())
    {
        return -4;
    }
    list();

    workingDir = QString::fromLocal8Bit(getWorkingDir());

    if(workingDir.isEmpty())
    {
        return -5;
    }
    return 0;
}

QString FtpClient::read_serv(long tv_sec, long tv_usec)
{
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(cs ,&fdr);
    timeval timeout;

    timeout.tv_sec = tv_sec;
    timeout.tv_usec = tv_usec;

    QString response;
    if(select(cs+1, &fdr, NULL, NULL, &timeout))
    {
        char buf[BUFLEN] = "";
        int readed = recv(cs, buf, BUFLEN - 1, 0);
        if(strlen(buf) != 0)
        {
            buf[readed] = '\0';
            response+=QString::fromLocal8Bit(buf);
        }
    }

    infoThread->sendInfo(response);

    return response;
}

int FtpClient::get(QString fileName, QString localPath)
{
    int file_size = getFileSize(fileName);

    if(file_size == -1)
    {
        closesocket(ds);
        infoThread->sendInfo("Reading server timeout\nCan't get response about file size: \n"
                             + fileName);
        return 1;
    }

    execCmd("RETR " + fileName);

    read_serv(0, 500000);

    ofstream fout((localPath + "/" + fileName).toLocal8Bit(), ios_base::binary);
    int read = 0;
    do
    {
        char dataBuff[DATABUFLEN];
        int readed = recv(ds, dataBuff, sizeof(dataBuff), 0);
        fout.write(dataBuff, readed);
        read +=readed;
    } while(read < file_size);

    fout.close();
    closesocket(ds);
    read_serv(0, 500000);
    return 0;
}

int FtpClient::getFileSize(QString fileName)
{
    execCmd("SIZE " + fileName);

    QString response = read_serv(0, 500000);

    if(response.isEmpty())
    {
        return -1;
    }

    int index = response.lastIndexOf(' ');
    response = response.right(response.size() - index);

    return response.toInt();
}

int FtpClient::login()
{
    execCmd("USER " + username);
    QString response = read_serv(0, 500000);
    if(response.isEmpty())
    {
        return -1;
    }

    execCmd("PASS " + password);
    response = read_serv(0, 500000);
    if(response.isEmpty())
    {
        return -1;
    }
    return 0;
}

int FtpClient::cd(QString dir)
{
    execCmd("CWD " + dir);

    QString response = read_serv(0, 500000);
    if(response.isEmpty())
    {
        infoThread->sendInfo("Can't get response\n");

        return 1;
    }else
    {
        int stateCode = getStateCode(response.toLocal8Bit().data());
        if(stateCode == 550)
        {
            return 2;
        }
    }

    if(init_dsock())
    {
        return 3;
    }

    if(list())
    {
        return 4;
    }
    workingDir = QString::fromLocal8Bit(getWorkingDir());
    if(workingDir.isEmpty())
    {
        return 5;
    }
    return 0;
}

int FtpClient::deleteFile(QString fileName)
{
    execCmd("DELE "+ fileName);
    read_serv(0, 500000);

    init_dsock();

    list();
    return 0;
}

int FtpClient::deleteDir(QString dirName)
{
    infoThread->sendInfo(dirName);

    execCmd("RMD "+ dirName);

    QString response = read_serv(0, 500000);
    int stateCode = getStateCode(response.toLocal8Bit().data());

    if(stateCode == 550)
    {
        return 1;
    }

    init_dsock();
    list();
    return 0;
}

int FtpClient::mkDir(QString name)
{
    execCmd("MKD " + name);

    QString response = read_serv(0, 500000);
    if(response.isEmpty())
    {
        infoThread->sendInfo("Can't get response\n");
        return 2;
    }
    int stateCode = getStateCode(response.toLocal8Bit().data());

    if(stateCode == 550)
    {
        return 1;
    }

    init_dsock();
    list();
    return 0;
}

int FtpClient::list()
{
    fullNameItemList.clear();
    shortNameItemList.clear();
    fileSizeList.clear();
    itemList.clear();


    execCmd("LIST");

    QString response = read_serv(0, 500000);
    if(response.isEmpty())
    {
        return -1;
    }

    int readed;
    QString list;
    while (true) {
        if(!waitForSocket(ds, 1, 0))
        {
            continue;
        }
        char buf[DATABUFLEN] = "";

        readed = recv(ds,buf,sizeof(buf) - 1,0);
        if (readed == 0 || readed == -1)
        {
            break;
        }

        buf[readed] = '\0';

        list+= QString::fromLocal8Bit(buf);

    }

    if(!list.isEmpty())
    {

        char *str = new char[list.length() + 1];

        strcpy(str, list.toLocal8Bit().data());

        char *lexptr = strtok(str, "\n");
        fullNameItemList.push_back(lexptr);

        do
        {
            lexptr = strtok(NULL, "\n");
            if (lexptr) fullNameItemList.push_back(lexptr);
        }while(lexptr);

        for(int i = 0; i < fullNameItemList.size(); i++)
        {
            char* lexptr;
            strtok(fullNameItemList[i], " ");
            for(int i = 0; i < 7; i++)
            {
                lexptr = strtok(NULL, " ");
                if(i == 3)
                {
                    fileSizeList.push_back(QString::fromLocal8Bit(lexptr));
                }
            }
            lexptr = strtok(NULL, "\r");

            while(*lexptr == ' ')
            {
                lexptr++;
            }

            shortNameItemList.push_back(lexptr);
        }
    }
    // FILL MY ITEM LIST

    if(shortNameItemList.size() >= 2)
    {
        if(strcmp(shortNameItemList.at(0), "..") && strcmp(shortNameItemList.at(1),".."))
        {
            itemList.push_back(ItemInfo("d", ".."));
        }
    }
    else if (shortNameItemList.size() == 1)
    {
        if(strcmp(shortNameItemList.at(0), ".."))
        {
            itemList.push_back(ItemInfo("d", ".."));
        }
    }
    else
    {
        itemList.push_back(ItemInfo("d", ".."));
    }

    for(int i = 0; i < fullNameItemList.size(); i++)
    {
        if(strcmp(shortNameItemList.at(i), "."))
        {
            if((fullNameItemList.at(i))[0] == 'd')
            {
                itemList.push_back(ItemInfo(QString::fromLocal8Bit(fullNameItemList.at(i)), QString::fromLocal8Bit(shortNameItemList.at(i))));
            }
            else
            {
                char *curr;
                char *prev;
                char *temp = new char[strlen(shortNameItemList.at(i)) + 1];

                strcpy(temp,shortNameItemList.at(i));
                curr = strtok(temp, ".");
                prev = curr;
                while((curr = strtok(NULL, ".")))
                {
                    prev = curr;
                }

                itemList.push_back(ItemInfo(QString::fromLocal8Bit(fullNameItemList.at(i)),
                                            fileSizeList.at(i),
                                            QString::fromLocal8Bit(shortNameItemList.at(i)),
                                            QString::fromLocal8Bit(prev)));
            }
        }
    }

    if(itemList.at(0).getShortName() != "..")
    {
        qSort((itemList).begin(), (itemList.end()), typeLessThan);
    }
    else if(itemList.size() > 1)
    {
        qSort((itemList).begin() + 1, (itemList.end()), typeLessThan);
    }

    closesocket(ds);
    read_serv(0, 500000);

    return 0;
}

int FtpClient::rename(QString src, QString dst)
{
    execCmd("RNFR "+ src);

    QString response_1 = read_serv(0, 500000);

    int stateCode_1 = getStateCode(response_1.toLocal8Bit().data());

    if(stateCode_1 == 550)
    {
        return 1;
    }else if(response_1.isEmpty())
    {
        infoThread->sendInfo("Can't get response\n");
        return 2;
    }

    execCmd("RNTO "+ dst);

    QString response_2 = read_serv(0, 500000);
    int stateCode_2 = getStateCode(response_2.toLocal8Bit().data());

    if(stateCode_2 == 550)
    {
        return 1;
    }else if(response_2.isEmpty())
    {
        infoThread->sendInfo("Can't get response\n");
        return 2;
    }

    init_dsock();
    list();
    return 0;
}

char *FtpClient::getWorkingDir()
{
    execCmd("PWD");

    QString resp = read_serv(0, 500000);

    char *resp_str = new char[resp.length() + 1];
    strcpy(resp_str, resp.toLocal8Bit().data());
    char *dir = strtok(resp_str,"\"");
    dir = strtok(NULL, "\"");

    return dir;
}

bool FtpClient::typeLessThan(const ItemInfo &i1, const ItemInfo &i2)
{
    return i1.isFile() < i2.isFile();
}

int FtpClient::sendFile(QString filePath, QString fileName)
{
    if(init_dsock())
    {
        infoThread->sendInfo("Error initializing data connection\n");
        return 4;
    }

    infoThread->sendInfo(fileName);

    execCmd("STOR " + fileName);

    QString response = read_serv(0, 500000);

    if(response.isEmpty())
    {
        infoThread->sendInfo("Reading server timeout\n");
        return 1;
    }

    int err_code = getStateCode(response.toLocal8Bit().data());
    if(err_code == 550)
    {
        infoThread->sendInfo("Can't send file\n");
        return 2;
    }

    ifstream fin(filePath.toLocal8Bit(),ios::in | ios::binary);
    if (!fin) {
        infoThread->sendInfo("Can't open file : source file wasn't recognized\n");
        return 3;
    }

    int read = 0;
    int sended = 0;
    do {
        char buff[DATABUFLEN];
        fin.read((char*)&buff,sizeof(buff));
        read = fin.gcount();
        if (read > 0)
        {
            sended = send(ds,buff,read,0);
        }
    } while(read > 0);
    fin.close();

    infoThread->sendInfo("File sended succesfully");

    closesocket(ds);
    read_serv(0, 500000);

    return 0;
}

int FtpClient::getStateCode(char *response)
{
    return atoi(strtok(response, " "));
}

int FtpClient::disconnect()
{
    execCmd("QUIT");

    read_serv(0, 500000);

    closesocket(cs);
    closesocket(ds);
    WSACleanup();
    return 0;
}

void FtpClient::setConnectData(QString ip_addr, QString username, QString password, QString port)
{
    this->ip_addr = ip_addr;
    this->username = username;
    this->password = password;
    this->port = port;
}

void FtpClient::forceDisconnect()
{
    closesocket(cs);
    closesocket(ds);
    WSACleanup();
}

void FtpClient::execCmd(QString cmd)
{
    cmd += "\r\n";
    send(cs, cmd.toLocal8Bit().data(), cmd.size(), 0);
}

FtpClient::~FtpClient()
{
    closesocket(cs);
    closesocket(ds);
    WSACleanup();

    delete infoThread;
}

int FtpClient::waitForSocket(SOCKET s, bool read, bool write)
{
    fd_set fdr;
    FD_ZERO(&fdr);
    FD_SET(s,&fdr);
    timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

    fd_set *second_param = nullptr;
    fd_set *third_param = nullptr;

    if(read)
    {
        second_param = &fdr;
    }
    else if (write)
    {
        third_param = &fdr;
    }

    return select(s+1, second_param, third_param, NULL, &timeout);
}
