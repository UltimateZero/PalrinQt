#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include <QObject>
#include "PalPacket.h"
#include "palrinqt_global.h"


class PALRINQTSHARED_EXPORT PacketBuilder : public QObject
{
    Q_OBJECT
public:
    explicit PacketBuilder(QObject *parent = 0);
    PalPacket LogonPacket(const QByteArray &email, int redirect = 0);
    PalPacket AuthPacket(const QByteArray &payload, int online_status = 1);
    PalPacket PingPacket();
    PalPacket ByePacket();
    PalPacket ContactDetailPacket();
    PalPacket GroupSubscribePacket(const QByteArray &name, const QByteArray &password);
    PalPacket GroupUnsubPacket(const QByteArray &group_id);
    PalPacket GroupAdminPacket(const QByteArray &group_id, const QByteArray &target_id, int action);
    PalPacket ContactAddResponsePacket(const QByteArray &contact_id, bool accepted);
    PalPacket ContactAddPacket(const QByteArray &user, const QByteArray &msg, bool isEmail);
    PalPacket ContactUpdatePacket(const QByteArray &contact_id);
    PalPacket SubProfileQueryPacket(const QByteArray &payload);
    PalPacket AvatarPacket(const QByteArray &id, int size, bool isGroup);
    PalPacket UrlPacket(const QByteArray &payload);
    PalPacket ProvifQueryPacket(const QByteArray &payload);
    PalPacket MesgPacket(const QByteArray &payload, const QByteArray &content_type, const QByteArray &target_id, int target_type);

    QByteArray buildDataMap(QByteArray head, QByteArray tail);

signals:

public slots:

private:
    QByteArray getBTR(const QByteArray &data);

};

#endif // PACKETBUILDER_H
