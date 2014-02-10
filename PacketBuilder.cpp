#include "PacketBuilder.h"

PacketBuilder::PacketBuilder(QObject *parent) :
    QObject(parent)
{
}

PalPacket PacketBuilder::LogonPacket(const QByteArray &email, int redirect)
{
    PalPacket packet;
    packet.addCommand("LOGON");

    packet.addHeader("Client-ID", email.toHex());
    packet.addHeader("Operator", "PC_CLIENT");
    packet.addHeader("affiliate-id", "winpc");
    packet.addHeader("app-type", "Windows x86");
    packet.addHeader("capabilities", "786437");
    packet.addHeader("client-version", "2.8.1, 60842");
    packet.addHeader("fw", "Win 6.2");
    packet.addHeader("last", "1");
    packet.addHeader("name", email);
    packet.addHeader("protocol-version", "2.0");
    packet.addHeader("redirect-count", QByteArray::number(redirect));

    return packet;
}

PalPacket PacketBuilder::AuthPacket(const QByteArray &payload, int online_status)
{
    PalPacket packet;
    packet.addCommand("AUTH");

    packet.addHeader("encryption-type", "1");
    packet.addHeader("online-status", QByteArray::number(online_status));

    packet.addPayload(payload);

    return packet;
}

PalPacket PacketBuilder::PingPacket()
{
    PalPacket packet;
    packet.addCommand("P");

    packet.addHeader("last","1");

    return packet;
}

PalPacket PacketBuilder::ByePacket()
{
    PalPacket packet;
    packet.addCommand("BYE");

    packet.addHeader("last","1");

    return packet;
}

PalPacket PacketBuilder::ContactDetailPacket()
{
    PalPacket packet;
    packet.addCommand("CONTACT DETAIL");

    return packet;
}

PalPacket PacketBuilder::GroupSubscribePacket(const QByteArray &name, const QByteArray &password)
{
    PalPacket packet;
    packet.addCommand("GROUP SUBSCRIBE");

    packet.addHeader("name", name);
    packet.addHeader("last", "1");

    packet.addPayload(password);

    return packet;
}

PalPacket PacketBuilder::GroupUnsubPacket(const QByteArray &group_id)
{
    PalPacket packet;
    packet.addCommand("GROUP UNSUB");

    packet.addHeader("group-id", group_id);

    return packet;
}

PalPacket PacketBuilder::GroupAdminPacket(const QByteArray &group_id, const QByteArray &target_id, int action)
{
    PalPacket packet;
    packet.addCommand("GROUP ADMIN");

    packet.addHeader("action", QByteArray::number(action));
    packet.addHeader("group-id", group_id);
    packet.addHeader("target-id", target_id);
    packet.addHeader("last", "1");

    return packet;
}

PalPacket PacketBuilder::ContactAddResponsePacket(const QByteArray &contact_id, bool accepted)
{
    PalPacket packet;
    packet.addCommand("CONTACT ADD RESP");

    packet.addHeader("source-id", contact_id);
    packet.addHeader("accepted", QByteArray::number(accepted));
    packet.addHeader("last", "1");

    return packet;
}

PalPacket PacketBuilder::ContactAddPacket(const QByteArray &user, const QByteArray &msg, bool isEmail)
{
    PalPacket packet;
    packet.addCommand("CONTACT ADD");

    packet.addHeader(isEmail ? "name" : "target-id", user);
    packet.addHeader("last", "1");

    packet.addPayload(msg);

    return packet;
}

PalPacket PacketBuilder::ContactUpdatePacket(const QByteArray &contact_id)
{
    PalPacket packet;
    packet.addCommand("CONTACT UPDATE");

    packet.addHeader("contact-id", contact_id);
    packet.addHeader("last", "1");

    return packet;
}

PalPacket PacketBuilder::SubProfileQueryPacket(const QByteArray &payload)
{
    PalPacket packet;
    packet.addCommand("SUB PROFILE QUERY");

    packet.addHeader("last", "1");

    packet.addPayload(payload);

    return packet;
}

PalPacket PacketBuilder::AvatarPacket(const QByteArray &id, int size, bool isGroup)
{
    PalPacket packet;
    packet.addCommand("AVATAR");

    packet.addHeader("id", id);
    packet.addHeader("size", QByteArray::number(size));
    packet.addHeader("group", QByteArray::number(isGroup));
    packet.addHeader("last", "1");

    return packet;
}

PalPacket PacketBuilder::UrlPacket(const QByteArray &payload)
{
    PalPacket packet;
    packet.addCommand("URL");

    packet.addHeader("last", "1");

    packet.addPayload(payload);

    return packet;
}

PalPacket PacketBuilder::ProvifQueryPacket(const QByteArray &payload)
{
    PalPacket packet;
    packet.addCommand("PROVIF QUERY");

    packet.addHeader("last", "1");

    packet.addPayload(payload);

    return packet;
}

PalPacket PacketBuilder::MesgPacket(const QByteArray &payload, const QByteArray &content_type, const QByteArray &target_id, int target_type)
{
    PalPacket packet;
    packet.addCommand("MESG");
    packet.addHeader("content-type", content_type);
    packet.addHeader("mesg-target", QByteArray::number(target_type));
    packet.addHeader("target-id", target_id);

    packet.addPayload(payload);

    return packet;
}


QByteArray PacketBuilder::buildDataMap(QByteArray head, QByteArray tail)
{
    head.append(char(0));
    head.append(getBTR(tail));
    head.append(tail);
    return head;
}

QByteArray PacketBuilder::getBTR(const QByteArray &data)
{
    QByteArray size;
    size.setNum(data.length(), 16);
    size = QByteArray::fromHex(size);
    if(size.length() < 2)
    {
        size.prepend(char(0));
    }

    return size;
}
