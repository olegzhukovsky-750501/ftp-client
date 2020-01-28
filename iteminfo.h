#pragma once

#include <QString>

class ItemInfo
{
public:
    ItemInfo();
    ItemInfo(QString fullName, QString fileSize, QString shortName, QString extension);
    ItemInfo(QString fullName, QString shortName);

    QString getFullName() const;
    QString getFileSize() const;
    QString getShortName() const;
    QString getExtension() const;
    bool isFile() const;
    bool isFolder() const;
private:
    QString fullName;
    QString fileSize;
    QString shortName;
    QString extension;
};
