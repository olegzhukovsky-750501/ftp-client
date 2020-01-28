#include "ftpsystemmodel.h"

FtpSystemModel::FtpSystemModel(const QVector<ItemInfo> &itemList, QObject *parent)
    :QAbstractTableModel(parent), itemList(itemList)
{
}

int FtpSystemModel::rowCount(const QModelIndex &parent) const
{
    return itemList.count();
}

int FtpSystemModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant FtpSystemModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }

    if(index.column() == 0)
    {
        switch(role)
        {
        case Qt::DisplayRole:
            return itemList.at(index.row()).getShortName();
        case Qt::DecorationRole:
            if(itemList.at(index.row()).isFile())
            {
                QFileInfo fileInfo(itemList.at(index.row()).getShortName());
                QFileIconProvider iconProvider;
                QIcon icon = iconProvider.icon(fileInfo);
                return icon;
            }
            else
            {
                QFileIconProvider fileIconProvider;
                return fileIconProvider.icon(QFileIconProvider::Folder);
            }
        default:
            return QVariant();
        }
    }
    else if(index.column() == 1)
    {
        switch(role)
        {
        case Qt::DisplayRole:
            if(itemList.at(index.row()).isFile())
            {
                return itemList.at(index.row()).getFileSize();
            }
            return QVariant();
        default:
            return QVariant();
        }
    }
}

bool FtpSystemModel::removeRows(int position, int rows, const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), position, position+rows - 1);

    for(int row = 0; row < rows; ++row)
    {
        itemList.removeAt(position);
    }

    endRemoveRows();

    return true;
}

QVariant FtpSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if(orientation == Qt::Horizontal)
    {
        if(section == 0)
        {
            return QString("Name");
        }
        else if(section == 1)
        {
            return QString("Size");
        }
    }
    else
    {
        return QString("Row %1").arg(section);
    }
}
