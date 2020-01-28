#include "infothread.h"

InfoThread::InfoThread()
{
}

void InfoThread::sendInfo(const QString &info) {
    emit emitInfo(info);
}
