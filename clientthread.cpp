#include "clientthread.h"

ClientThread::ClientThread() {
    for(int i=0; i<2; i++)
    {
        arglist.push_back("");
    }
    client = new FtpClient();

}

ClientThread::~ClientThread() {
    delete client;
}


void ClientThread::run() {
    switch (task) {
    case Connect:
        if(!client->establishConnect())
        {
            fillList();
            emit emitSuccess();    
        }
        else
        {
            client->forceDisconnect();
            emit emitSuccess();
        }
        break;
    case Disconnect:
        if(!client->disconnect())
        {
            emit emitSuccess();
        }
        break;
    case ChangeDir:
        if(client->cd(arglist[0]))
        {
          arglist[0] = "BAD";
          fillList();
          client->forceDisconnect();
          emit emitSuccess();
        }
        else
        {
            fillList();
        }
        break;
    case Download:
        client->init_dsock();
        client->get(arglist[0], arglist[1]);
        break;
    case Upload:
        if(client->sendFile(arglist[0], arglist[1]))
        {
            break;
        }
        client->init_dsock();
        client->list();
        fillList();
        break;
    case RemoveFile:
        if(!client->deleteFile(arglist[0]))
        {
            fillList();
        }
        break;
    case RemoveDir:
        if(!client->deleteDir(arglist[0]))
        {
            fillList();
        }
        break;
    case Rename:
        if(!client->rename(arglist[0], arglist[1]))
        {
            fillList();
        }
        break;
    case MakeDir:
        if(!client->mkDir(arglist[0]))
        {
            fillList();
        }
        break;
    default:
        break;
    }

}

void ClientThread::stop()
{
    isInterruptionRequested();
    quit();
    wait();
}

void ClientThread::fillList() {
    if(!(arglist[0] == "BAD"))
    {
        emit emitClearList();
    }
    emit emitFillList();
}

