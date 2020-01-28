#pragma once

#include <QThread>
#include <QString>

class InfoThread : public QThread
{
    Q_OBJECT
public:
    explicit InfoThread();
    void sendInfo(const QString &info);

signals:
    void emitInfo(const QString &info);
};
