#include "PalMessage.h"

PalMessage::PalMessage(PalPacket packet)
{
    msgPacket = packet;
}

QByteArray PalMessage::body()
{
    return msgPacket.getPayload();
}

QString PalMessage::contentType()
{
    return msgPacket.getHeader("CONTENT-TYPE");
}

int PalMessage::targetType()
{
    if(msgPacket.hasHeader("TARGET-ID"))
        return PalMessage::Groupchat;
    else
        return PalMessage::Chat;
}

QString PalMessage::email()
{
    return QString::fromUtf8(msgPacket.getHeader("EMAIL"));
}

QString PalMessage::name()
{
    return QString::fromUtf8(msgPacket.getHeader("NAME"));
}

QString PalMessage::from()
{
    return msgPacket.getHeader("SOURCE-ID");
}

QString PalMessage::group()
{
    return msgPacket.getHeader("TARGET-ID");
}

bool PalMessage::isLast()
{
    return msgPacket.isLast();
}

int PalMessage::id()
{
    return msgPacket.getHeader("MESG-ID").toInt();
}

qlonglong PalMessage::timeStamp()
{
    return msgPacket.getHeader("TIMESTAMP").toLongLong();
}
