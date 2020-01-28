#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setFixedSize(1200,720);
    //INIT LOCAL FILE SYSTEM
    ftpModel = nullptr;
    localModel = new QFileSystemModel(this);
    localModel->setFilter(QDir::AllEntries | QDir::NoDot);
    localModel->setRootPath(""); //TRACK ALL CHANGES
    ui->tableView->setModel(localModel);
    QDir dir;
    ui->comboBox->addItem("/");
    prevPath = "/";
    foreach(QFileInfo var, dir.drives()) //ADDING DRIVES TO COMBOBOX
    {
        ui->comboBox->addItem(var.absoluteFilePath());
    }


    //CONNECTING SIGNALS CLIENTTHREAD -----> MAINWINDOW SLOTS

    clientThread = new ClientThread();

    connect(clientThread, SIGNAL(emitFillList()), this, SLOT(recvFillList()));
    connect(clientThread, SIGNAL(emitSuccess()), this, SLOT(recvSuccess()));
    connect(clientThread, SIGNAL(finished()), clientThread, SLOT(stop()));
    connect(clientThread, SIGNAL(emitClearList()), this, SLOT(recvClearList()));
    connect(clientThread->client->infoThread, SIGNAL(emitInfo(const QString)), this, SLOT(recvInfo(const QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(index.column() != 0)
    {
        return;
    }
    QTableView* tableView = (QTableView*)sender();

    QFileInfo fileInfo = localModel->fileInfo(index);


    if(fileInfo.isFile())
    {
        return;
    }
    else if (fileInfo.fileName() == "..")
    {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        tableView->setRootIndex(localModel->index(dir.absolutePath()));
    }
    else if (fileInfo.isDir())
    {
        tableView->setRootIndex(index);

        int index = ui->comboBox->findText(fileInfo.canonicalFilePath());
        if(index == -1)
        {
            ui->comboBox->addItem(fileInfo.canonicalFilePath());
        }
    }
    ui->comboBox->setCurrentText(fileInfo.canonicalFilePath());

    QStorageInfo info(fileInfo.dir());

    ui->label_6->setText("");

    prevPath = fileInfo.canonicalFilePath();
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    QFileInfo fileInfo = localModel->fileInfo(localModel->index(arg1));
    if(fileInfo.path().isEmpty() && (arg1 != "/"))
    {
        int index = ui->comboBox->findText(arg1);
        if(index != -1)
        {
            ui->comboBox->removeItem(index);
        }

        ui->comboBox->setCurrentText(prevPath);
        QMessageBox msgBox;
        msgBox.setText("'" + arg1 + "'" + " doesn't exist or unavailiable");
        msgBox.exec();
        return;
    }
    ui->tableView->setRootIndex(localModel->index(arg1));

    prevPath = arg1;

}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    QFileInfo fileInfo = localModel->fileInfo(index);
    if(fileInfo.isFile())
    {
        ui->label_6->setText("Size of selected file: "
                             + QString::number(fileInfo.size())
                             + " bytes");
    }
    else {
        ui->label_6->setText("");
    }
    ui->tableView_2->clearSelection();
}



void MainWindow::on_pushButton_clicked()
{
    if(!clientThread->isRunning())
    {
        if(isConnected)
        {
            clientThread->task = Disconnect;
            clientThread->start();
        }
        else
        {
            if(ui->lineEdit->text().isEmpty())
            {
                QMessageBox msgBox;
                msgBox.setText("Can't recognize host");
                msgBox.exec();
                return;
            }

            QString ip_addr = ui->lineEdit->text();

            QString username = ui->lineEdit_2->text();
            QString password = ui->lineEdit_4->text();
            QString port = ui->lineEdit_3->text();

            if(username.isEmpty())
            {
                username = "anonymous";
            }

            if(password.isEmpty())
            {
                password = "anonymous";
            }

            if(port.isEmpty())
            {
                port = "21";
            }

            clientThread->client->setConnectData(ip_addr, username, password, port);
            clientThread->task = Connect;
            clientThread->start();
        }
    }
}

void MainWindow::on_tableView_2_doubleClicked(const QModelIndex &index)
{
    QString str = index.model()->data(index, Qt::DisplayRole).toString();


    if(itemList.at(index.row()).isFolder())
    {
        if(!clientThread->isRunning()) {
            clientThread->arglist[0] = str;
            clientThread->task = ChangeDir;
            clientThread->start();
        }
    }
}

void MainWindow::recvFillList()
{
    if(clientThread->arglist[0] == "BAD")
    {
        ui->comboBox_2->setCurrentText(clientThread->client->workingDir);
        clientThread->arglist[0] = "";
        return;
    }
    itemList = clientThread->client->itemList;

    int index = ui->comboBox_2->findText(clientThread->client->workingDir);

    if(index == -1)
    {
        ui->comboBox_2->addItem(clientThread->client->workingDir);
    }

    ui->comboBox_2->setCurrentText(clientThread->client->workingDir);

    ftpModel = new FtpSystemModel(itemList, this);
    ui->tableView_2->setModel(ftpModel);
}

void MainWindow::recvClearList()
{
    itemList.clear();
    if(ftpModel != nullptr)
    {
        ftpModel->removeRows(0, ftpModel->rowCount());
        ftpModel = nullptr;
    }
}

void MainWindow::recvInfo(const QString &info)
{
    ui->textEdit->append(info);
}

void MainWindow::recvSuccess()
{
    if(isConnected)
    {
        ui->tableView_2->setEnabled(false);
        ui->tableView_2->horizontalHeader()->setVisible(false);
        ui->comboBox_2->clear();
        ui->comboBox_2->setEnabled(false);
        recvClearList();
        isConnected = false;
        ui->pushButton->setText("Connect");

        QMessageBox msgBox;
        msgBox.setText("Disconnected");
        msgBox.exec();
    }
    else
    {
        ui->tableView_2->setEnabled(true);
        ui->tableView_2->horizontalHeader()->setVisible(true);
        ui->comboBox_2->addItem("/");
        ui->comboBox_2->setEnabled(true);
        isConnected = true;
        ui->pushButton->setText("Disconnect");

        QMessageBox msgBox;
        msgBox.setText("Succesfull connection");
        msgBox.exec();
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if(ui->tableView_2->currentIndex().row() == -1)
    {
        QMessageBox msgBox;
        msgBox.setText("Select file at ftp server to transer");
        msgBox.exec();
    }
    else if(prevPath == "/")
    {
        QMessageBox msgBox;
        msgBox.setText("Open destination folder");
        msgBox.exec();
    }
    else
    {
        if(itemList.at(ui->tableView_2->currentIndex().row()).isFile())
        {
            if(!clientThread->isRunning())
            {
                clientThread->arglist[0] = itemList.at(ui->tableView_2->currentIndex().row()).getShortName();
                clientThread->arglist[1] = prevPath;
                clientThread->task = Download;
                clientThread->start();
            }
        }
        else
        {
            return;
        }
    }
}


void MainWindow::on_tableView_2_clicked(const QModelIndex &index)
{
    ui->tableView->clearSelection();
}

void MainWindow::on_pushButton_2_clicked()
{
    if(ui->tableView->currentIndex().row() == -1)
    {
        QMessageBox msgBox;
        msgBox.setText("Select your file to transer to FTP server");
        msgBox.exec();
    }
    else
    {
        if(!clientThread->isRunning())
        {
            QFileInfo fileInfo = localModel->fileInfo(ui->tableView->currentIndex());
            if(fileInfo.isFile())
            {
                clientThread->arglist[0] = fileInfo.canonicalFilePath();
                clientThread->arglist[1] = fileInfo.fileName();
                clientThread->task = Upload;
                clientThread->start();
            }
        }
    }
}

void MainWindow::on_comboBox_2_activated(const QString &arg1)
{
    if(!clientThread->isRunning())
    {
        if(arg1 == clientThread->client->workingDir)
        {
            return;
        }

        clientThread->arglist[0] = arg1;
        clientThread->task = ChangeDir;
        clientThread->start();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About Light FTP Client", "Light FTP Client is implemented on sockets using WinSock2 library."
                                                       "\nThis program was developed for a course project at BSUIR. 2019");
}

void MainWindow::on_actionDelete_triggered()
{
    if(!clientThread ->isRunning())
    {
        if(isConnected && ui->tableView_2->currentIndex().row() != -1)
        {
            if((itemList.at(ui->tableView_2->currentIndex().row()).isFolder())
               && (itemList.at(ui->tableView_2->currentIndex().row()).getShortName() != ".."))
            {
                clientThread->arglist[0] = itemList.at(ui->tableView_2->currentIndex().row()).getShortName();
                clientThread->task = RemoveDir;
                clientThread->start();
            }
            else if(itemList.at(ui->tableView_2->currentIndex().row()).isFile())
            {
                clientThread->arglist[0] = itemList.at(ui->tableView_2->currentIndex().row()).getShortName();
                clientThread->task = RemoveFile;
                clientThread->start();
            }
        }
    }
}

void MainWindow::on_actionRename_triggered()
{
    if(!clientThread ->isRunning())
    {
        if(isConnected && ui->tableView_2->currentIndex().row() != -1)
        {
            if((itemList.at(ui->tableView_2->currentIndex().row()).getShortName() != ".."))
            {
                QString dstName = QInputDialog::getText(this, "Please input a name", "New name of the file");

                if(dstName.isEmpty())
                {
                    return;
                }
                if(dstName=="." || dstName=="..")
                {
                    return;
                }
                clientThread->arglist[0] = itemList.at(ui->tableView_2->currentIndex().row()).getShortName();
                clientThread->arglist[1] = dstName;
                clientThread->task = Rename;
                clientThread->start();
            }
        }
    }
}

void MainWindow::on_actionAdd_triggered()
{
    if(!clientThread ->isRunning())
    {
        if(isConnected)
        {
            QString name = QInputDialog::getText(this, "Please input a name", "Name of new directory");

            if(name.isEmpty())
            {
                return;
            }
            if(name=="." || name=="..")
            {
                return;
            }

            clientThread->arglist[0] = name;
            clientThread->task = MakeDir;
            clientThread->start();
        }
    }
}
