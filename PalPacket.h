#ifndef PALPACKET_H
#define PALPACKET_H

#include <QByteArray>
#include <QHash>
#include <QStringList>
#include <string>

#include "palrinqt_global.h"

using namespace std;


class PALRINQTSHARED_EXPORT PalPacket
{
public:
    PalPacket(bool full = true);


    void addCommand(QByteArray cmd);
    QByteArray getCommand();

    void addHeader(QByteArray header, QByteArray value);
    QByteArray getHeader(QByteArray header);

    QHash<QByteArray, QByteArray> getHeaders();

    void setFull(bool full);
    int getPayloadSize();

    void addPayload(QByteArray pl);
    QByteArray getPayload();
    QByteArray getCompressedPayload();
    void appendPayload(QByteArray pl);
    void prependPayload(QByteArray pl);

    QByteArray getRawPacket();
    QByteArray getWriteablePacket();

    bool fromRawData(QByteArray data);

    bool isValid();
    bool isLast();
    bool hasHeader(QByteArray header);
    int mesgId();


private:
    QByteArray command;
    QHash<QByteArray, QByteArray> headers;
    QByteArray payload;
    QByteArray compressedPayload;
    bool isFull;
    bool is_valid;


private:
    QByteArray decompressPayload(const QByteArray &compressedPayload);


};

#endif // PALPACKET_H
