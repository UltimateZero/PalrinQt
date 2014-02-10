#include "PalPacket.h"
#include "CryptoManager.h"

#include <QDebug>

PalPacket::PalPacket(bool full)
{
    isFull = full;
    is_valid = true;
}



void PalPacket::addCommand(QByteArray cmd)
{
    command = cmd;
}

QByteArray PalPacket::getCommand()
{
    return command;
}

void PalPacket::addHeader(QByteArray header, QByteArray value)
{
    if(value != "0" || header.toUpper() == "MESG-TARGET")
        headers[header.toUpper()] = value;
}

QByteArray PalPacket::getHeader(QByteArray header)
{
    if(headers.keys().contains(header.toUpper())) //avoid creating the key
        return headers[header.toUpper()];
    else
        return "0";
}

QHash<QByteArray, QByteArray> PalPacket::getHeaders()
{
    return headers;
}

void PalPacket::setFull(bool full)
{
    isFull = full;
}

int PalPacket::getPayloadSize()
{
    return getCompressedPayload().size();
}

void PalPacket::addPayload(QByteArray pl)
{
    payload = pl;
    compressedPayload = pl;
    if(isFull)
    {
        if(headers.contains("COMPRESSION"))
            payload = decompressPayload(payload);
        if(headers.contains("IV"))
        {
            payload = payload.mid(headers["IV"].toInt());
        }
    }


}

QByteArray PalPacket::getPayload()
{
    return payload;
}

QByteArray PalPacket::getCompressedPayload()
{
    return compressedPayload;
}

void PalPacket::appendPayload(QByteArray pl)
{
    pl = compressedPayload + pl;
    addPayload(pl);
}

void PalPacket::prependPayload(QByteArray pl)
{
    pl = pl + compressedPayload;
    addPayload(pl);
}

QByteArray PalPacket::getRawPacket()
{
    QByteArray out;
    QByteArray endLine = "\r\n";

    out.append(command);
    out.append(endLine);
    foreach(QByteArray header, headers.keys())
    {
        out.append( header + ": " + headers[header] );
        out.append(endLine);
    }
    out.append(endLine);
    out.append(compressedPayload);

    return out;
}



bool PalPacket::fromRawData(QByteArray data)
{
    QStringList dataList = QString(data.toHex()).split("0d0a0d0a");

    if(dataList.length() < 2)
    {
        qDebug() << "MALFORMED PACKET";
        qDebug() << data.toHex();
        is_valid = false;
        return false;
    }


    QStringList headList = dataList.takeFirst().split("0d0a");
    addCommand(QByteArray::fromHex(headList.takeFirst().toAscii()).trimmed());

    foreach(QString headData, headList)
    {
        headData = QByteArray::fromHex(headData.toAscii()).trimmed();
        addHeader(headData.split(": ")[0].toUpper().toAscii().trimmed(), headData.split(": ")[1].toAscii().trimmed());
    }

    addPayload(QByteArray::fromHex(dataList.join("0d0a0d0a").toAscii()));

    return true;
}

bool PalPacket::isValid()
{
    return is_valid;
}

bool PalPacket::isLast()
{
    if(hasHeader("LAST"))
    {
        return true;
    }
    else if(hasHeader("CORRELATION-ID"))
    {
        return false;
    }
    else if(hasHeader("TOTAL-LENGTH"))
    {
        int total_length = getHeader("TOTAL-LENGTH").toInt();
        int content_length = getHeader("CONTENT-LENGTH").toInt();
        if(total_length != content_length)
            return false;
        else
            return true;
    }
    else
    {
        if(getCommand() == "URL")
            return true;
        else
            return false;
    }

}

bool PalPacket::hasHeader(QByteArray header)
{
    return headers.keys().contains(header.toUpper());
}

int PalPacket::mesgId()
{
    if(!hasHeader("MESG-ID"))
        return -1;
    return getHeader("MESG-ID").toInt();
}

QByteArray PalPacket::decompressPayload(const QByteArray &compressedPayload)
{
    string str(compressedPayload.constData(), compressedPayload.length());
    string out = CryptoManager::zlibUncompress(str);
    QByteArray uncompressedPayload = QString::fromStdString(out).toAscii();

    return uncompressedPayload;
}

