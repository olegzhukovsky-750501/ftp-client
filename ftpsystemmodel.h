#pragma once

#include <QAbstractTableModel>
#include <QApplication>
#include <QFileIconProvider>
#include <QIcon>
#include <QStyle>
#include <QDebug>
#include <vector>
#include "iteminfo.h"

using namespace std;

class FtpSystemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FtpSystemModel(const QVector<ItemInfo> &items, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
private:
    QVector<ItemInfo> itemList;
};
