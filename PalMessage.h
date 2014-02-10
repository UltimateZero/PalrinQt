#ifndef PALMESSAGE_H
#define PALMESSAGE_H

#include <QObject>
#include "PalPacket.h"
#include "palrinqt_global.h"

class PALRINQTSHARED_EXPORT PalMessage
{

public:
    PalMessage(PalPacket packet);

    enum TargetTypes {
        Chat = 0,
        Groupchat
    };

    QByteArray body();
    QString contentType();
    int targetType();
    QString email();
    QString name();
    QString from();
    QString group();
    bool isLast();
    int id();
    qlonglong timeStamp();


private:
    PalPacket msgPacket;


};

#endif // PALMESSAGE_H
