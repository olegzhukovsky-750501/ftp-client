#include "iteminfo.h"

ItemInfo::ItemInfo()
{
}

ItemInfo::ItemInfo(QString fullName, QString fileSize, QString shortName, QString extension)
    :fullName(fullName),fileSize(fileSize), shortName(shortName), extension(extension)
{
}

ItemInfo::ItemInfo(QString fullName, QString shortName)
    :fullName(fullName), shortName(shortName)
{
}

QString ItemInfo::getFullName() const
{
    return fullName;
}

QString ItemInfo::getFileSize() const
{
    return fileSize;
}

QString ItemInfo::getShortName() const
{
    return shortName;
}

QString ItemInfo::getExtension() const
{
    return extension;
}

bool ItemInfo::isFile() const
{
    if(fullName[0] == 'd')
    {
        return false;
    }
    return true;
}

bool ItemInfo::isFolder() const
{
    return !isFile();
}
