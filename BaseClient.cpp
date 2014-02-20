#include "BaseClient.h"
#include "DataMapParser.h"
#include "json.h"
#include <QDebug>

BaseClient::BaseClient(QObject *parent) :
    QObject(parent)
{
    conn = new ConnectionManager(this);
    reconnect_timer = new QTimer(this);
    utils = new Utils(this);
    reconnect_timer->setInterval(1000*3);
    reconnect_timer->setSingleShot(true);
    config["auto_reconnect"] = true;
    config["store_data"] = true;

    registerSignals();
}

BaseClient::~BaseClient()
{
    delete conn;
    delete reconnect_timer;
    delete utils;
}

void BaseClient::connectToHost(const QByteArray &email, const QByteArray &password)
{
    config["email"] = email;
    config["password"] = password;
    conn->startConnection(email, password);
}

void BaseClient::connectToHost()
{
    if(config["email"].toString().isEmpty())
    {
        qDebug() << "No configuration";
        return;
    }
    conn->startConnection(config["email"].toByteArray(), config["password"].toByteArray());
}

void BaseClient::setEmail(const QByteArray &email)
{
    config["email"] = email;
}

void BaseClient::setPassword(const QByteArray &password)
{
    config["password"] = password;
}

void BaseClient::setAutoReconnected(bool autoreconnect)
{
    config["auto_reconnect"] = autoreconnect;
}

void BaseClient::setStoreData(bool storedata)
{
    config["store_data"] = storedata;
}

void BaseClient::setPrintCmds(bool print)
{
    conn->setPrintCmds(print);
}

QByteArray BaseClient::getSelfId()
{
    return self["sub-id"].toByteArray();
}

int BaseClient::sendMessage(const QString &id, QString body, int target_type)
{
    return conn->sendMessage(target_type, id.toUtf8(), "text/plain", body.toUtf8());
}

int BaseClient::sendImage(const QString &id, QByteArray body, int target_type)
{
    return conn->sendMessage(target_type, id.toUtf8(), "image/jpeg", body);
}

int BaseClient::sendAudio(const QString &id, QByteArray body, int target_type)
{
    return conn->sendMessage(target_type, id.toUtf8(), "audio/x-speex", body);
}

int BaseClient::joinGroup(const QByteArray &group_name, QByteArray group_pw)
{
    return conn->joinGroup(group_name, group_pw);
}

int BaseClient::leaveGroup(const QByteArray &group_id)
{
    groups.remove(group_id);
    return conn->leaveGroup(group_id);
}

int BaseClient::kickUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("KICK", true).toInt());
}

int BaseClient::silenceUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("SILENCE", true).toInt());
}

int BaseClient::resetUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("RESET", true).toInt());
}

int BaseClient::banUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("BAN", true).toInt());
}

int BaseClient::modUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("MOD", true).toInt());
}

int BaseClient::adminUser(const QByteArray &group_id, const QByteArray &target_id)
{
    return conn->sendGroupAction(group_id, target_id, utils->groupAction("ADMIN", true).toInt());
}

int BaseClient::addContact(const QByteArray &user, bool isEmail, QByteArray msg)
{
    return conn->addContact(user, isEmail, msg);
}

int BaseClient::removeContact(const QByteArray &contact_id)
{
    return conn->removeContact(contact_id);
}

int BaseClient::acceptContact(const QByteArray &contact_id)
{
    return conn->acceptContact(contact_id);
}

int BaseClient::rejectContact(const QByteArray &contact_id)
{
    return conn->rejectContact(contact_id);
}

int BaseClient::getContactProfile(const QByteArray &contact_id)
{
    return conn->getContactProfile(contact_id);
}

int BaseClient::getAvatar(const QByteArray &id, int size, bool isGroup)
{
    return conn->getAvatar(id, size, isGroup);
}

int BaseClient::getGroupInfo(const QByteArray &group, bool isName)
{
    return conn->getGroupInfo(group, isName);
}

QHash<QString, QVariant> BaseClient::getStoredContactData(const QString &contact_id)
{
    return contacts[contact_id];
}

QHash<QString, QVariant> BaseClient::getStoredGroupData(const QString &group_id)
{
    return groups[group_id];
}

QHash<QString, QVariant> BaseClient::getStoredSelfData()
{
    return self;
}



void BaseClient::connectedToHost()
{
    qDebug() << "Logged in as " << config["email"].toByteArray();
    emit connected();
}

void BaseClient::disconnectedFromHost(bool error, QByteArray reason)
{
    if(error)
    {
        qDebug() << "Disconnected on error: " << reason;
        emit disconnected(reason);
    }
    else if(reason == "Redirected");
    else if(reason == "Ghosted")
    {
        emit disconnected("Logged in from another location");
    }
    else
    {
        qDebug() << "Client disconnected";
        if(config["auto_reconnect"].toBool())
        {
            qDebug() << "Reconnect timer started";
            reconnect_timer->start();
        }
        else
        {
            emit disconnected(reason);
        }
    }

}

void BaseClient::reconnect()
{
    qDebug() << "Reconnecting...";
    connectToHost();
}

void BaseClient::messageReceived(PalPacket packet)
{
    if(!isLastPacket(&packet))
        return;

    PalMessage msg(packet);
    emit messageReceived(msg);
}

void BaseClient::groupAdminReceived(PalPacket packet)
{
    QByteArray group_id = packet.getHeader("GROUP-ID");
    QByteArray source_id = packet.getHeader("SOURCE-ID");
    QByteArray target_id = packet.getHeader("TARGET-ID");
    QByteArray action = utils->groupAction(packet.getHeader("ACTION"));

    emit groupAdminReceived(group_id, source_id, target_id, action);
}

void BaseClient::groupUpdateReceived(PalPacket packet)
{
    QHash<QString, QVariant> all = DataMapParser::getDataMap(packet.getPayload());

    QByteArray group_id = all["group-id"].toByteArray();
    QByteArray contact_id = all["contact-id"].toByteArray();
    int update_type = all["type"].toInt();
    if(update_type == 0)
        parseContacts(all);
    emit groupUpdateReceived(group_id, contact_id, update_type, all["contacts"].toHash());

}

void BaseClient::ownSubProfileReceived(PalPacket packet)
{
    subProfileReceived(packet);
    QHash<QString, QVariant> all = DataMapParser::getDataMap(packet.getPayload());

    all.remove("contacts"); all.remove("group_sub"); all.remove("contact_add");

    if(config["store_data"].toBool())
        updateSelf(all);
    emit selfDataReceived(all);
    self["sub-id"] = all["sub-id"].toByteArray();
    qDebug() << "Self Id:" << self["sub-id"].toByteArray();
}

void BaseClient::subProfileQueryResultReceived(PalPacket packet)
{
    subProfileReceived(packet);
    QHash<QString, QVariant> all = DataMapParser::getDataMap(packet.getPayload());
    QByteArray id = all["sub-id"].toByteArray();
    updateContact(id, all);

    if(!config["store_data"].toBool())
    {
        emit contactProfileReceived(contacts.take(id), id);
    }
    else
    {
        emit contactProfileReceived(contacts[id], id);
    }

}

void BaseClient::subProfileReceived(PalPacket packet)
{
    QHash<QString, QVariant> all = DataMapParser::getDataMap(packet.getPayload());

    parseContacts(all);

    parseGroups(all);

    parseAdds(all);
}

void BaseClient::avatarReceived(PalPacket packet)
{
    if(!isLastPacket(&packet))
        return;
    emit avatarReceived(packet.getPayload(), packet.mesgId());
}

void BaseClient::urlReceived(PalPacket packet)
{
    if(!isLastPacket(&packet))
        return;

    PalPacket temp;
    temp.fromRawData(packet.getPayload());
    qDebug() << "Packet payload:" << packet.getPayload();
    qDebug() << "URL payload:" << temp.getPayload();

    if(temp.getHeader("CONTENT-TYPE") == "application/json")
    {   Json json;
        QMap<QString, QVariant> json_map = json.decode(temp.getPayload());
        emit urlDataReceived(json_map, packet.mesgId());
    }
    else
    {
        emit urlDataReceived(temp.getPayload(), packet.mesgId());
    }
}

void BaseClient::responseReceived(PalPacket packet)
{
    int type = packet.getHeader("TYPE").toInt();
    int what = packet.getHeader("WHAT").toInt();
    int code = packet.getPayload().toHex().toInt(0, 16);
    qDebug() << "RESPONSE ID:" << packet.mesgId() << "TYPE:" << type << "WHAT:" << what << "CODE:" << code;

}

void BaseClient::unknownPacketReceived(PalPacket packet)
{
    qDebug() << "Unknown command received:" << packet.getCommand();
}


/////////////////////////////////////////

void BaseClient::updateContact(const QString &contact_id, const QHash<QString, QVariant> &contact_data)
{
    foreach(QString data_type, contact_data.keys())
    {
        if(data_type.contains("."))
        {
            contacts[contact_id][data_type.split(".")[0]].toHash()[data_type.split(".")[1]] = contact_data[data_type];
        }
        else
            contacts[contact_id][data_type] = contact_data[data_type];
    }

    if(contacts[contact_id].keys().contains("contact") && !contacts_list.contains(contact_id))
    {
        contacts_list << contact_id;
        qDebug() << "Found contacts list contact id " << contact_id;
    }
    emit contactDataReceived(contact_id, contact_data);
}

void BaseClient::updateGroup(const QString &group_id, const QHash<QString, QVariant> &group_data)
{
    foreach(QString data_type, group_data.keys())
    {
        if(data_type.contains("."))
        {
            groups[group_id][data_type.split(".")[0]].toHash()[data_type.split(".")[1]] = group_data[data_type];
        }
        else
            groups[group_id][data_type] = group_data[data_type];
    }
    qDebug() << "data updated for group #" << group_id;
    emit groupDataReceived(group_id, group_data);
}

void BaseClient::updateSelf(const QHash<QString, QVariant> &data)
{
    foreach(QString data_type, data.keys())
    {
        if(data_type.contains("."))
        {
            self[data_type.split(".")[0]].toHash()[data_type.split(".")[1]] = data[data_type];
        }
        else
            self[data_type] = data[data_type];
    }
    emit selfDataReceived(data);
}

QList<QByteArray> BaseClient::parseContacts(const QHash<QString, QVariant> &all, bool force)
{
    QList<QByteArray> ids;
    if(all.keys().contains("contacts")) //contacts
    {
        QHash<QString, QVariant> data = all["contacts"].toHash();
        foreach(QString contact_id, data.keys())
        {
            ids.append(contact_id.toUtf8());
            QHash<QString, QVariant> contact_data = data[contact_id].toHash();
            if(!contact_id.isEmpty())
            {
                if(config["store_data"].toBool() || force)
                    updateContact(contact_id, contact_data);
                else
                    emit contactDataReceived(contact_id, contact_data);
            }
            else
            {
                contact_data.remove("sub-id");
                if(config["store_data"].toBool() || force)
                    updateSelf(contact_data);
                else
                    emit selfDataReceived(contact_data);
            }
        }
    }
    return ids;
}

QList<QByteArray> BaseClient::parseGroups(const QHash<QString, QVariant> &all)
{
    QList<QByteArray> ids;
    if(all.keys().contains("group_sub")) //groups roster
    {
        QHash<QString, QVariant> data = all["group_sub"].toHash();
        foreach(QString group_id, data.keys())
        {
            ids.append(group_id.toUtf8());
            QHash<QString, QVariant> group_data = data[group_id].toHash();
            if(config["store_data"].toBool())
                updateGroup(group_id, group_data);
            else
                emit groupDataReceived(group_id, group_data);
        }
    }
    return ids;
}

QList<QByteArray> BaseClient::parseAdds(const QHash<QString, QVariant> &all)
{
    QList<QByteArray> ids;
    if(all.keys().contains("contact_add")) //add requests
    {
        QHash<QString, QVariant> data = all["contact_add"].toHash();
        foreach(QString contact_id, data.keys())
        {
            ids.append(contact_id.toUtf8());
            QHash<QString, QVariant> contact_data = data[contact_id].toHash();
            if(config["store_data"].toBool())
                updateContact(contact_id, contact_data);
            emit contactAddRequestReceived(contact_id, contact_data);
        }
    }
    return ids;
}

bool BaseClient::isLastPacket(PalPacket *packet)
{
    int id = packet->getHeader("MESG-ID").toInt();
    int correlation_id = packet->hasHeader("CORRELATION-ID") ? packet->getHeader("CORRELATION-ID").toInt() : id;

    if(!packet->isLast())
    {
//        qDebug() << "incomplete packet" << packet->hasHeader("TOTAL-LENGTH");
        if(incompletePackets.contains(correlation_id))
        {
            incompletePackets[correlation_id].second += packet->getCompressedPayload();
        }
        else
        {
            incompletePackets[correlation_id].second = packet->getCompressedPayload();
            incompletePackets[correlation_id].first = packet->getHeader("TOTAL-LENGTH").toInt();
        }
        emit packetPartReceived(correlation_id, utils->toPercent(incompletePackets[correlation_id].second.size(), incompletePackets[correlation_id].first));
        return false;
    }
    else
    {
        if(incompletePackets.contains(correlation_id))
        {
            packet->setFull(true);
            packet->prependPayload(incompletePackets[correlation_id].second);
            packet->addHeader("TOTAL-LENGTH", QByteArray::number(packet->getPayloadSize()));
            emit packetPartReceived(correlation_id, utils->toPercent(packet->getPayloadSize(), incompletePackets[correlation_id].first));
            incompletePackets.remove(correlation_id);
        }
        return true;
    }

}

void BaseClient::registerSignals()
{
    connect(reconnect_timer, SIGNAL(timeout()), this, SLOT(reconnect()));

    connect(conn, SIGNAL(connected()), this, SLOT(connectedToHost()));
    connect(conn, SIGNAL(disconnected(bool,QByteArray)), this, SLOT(disconnectedFromHost(bool,QByteArray)));
    connect(conn, SIGNAL(messageReceived(PalPacket)), this, SLOT(messageReceived(PalPacket)));
    connect(conn, SIGNAL(groupAdminReceived(PalPacket)), this, SLOT(groupAdminReceived(PalPacket)));
    connect(conn, SIGNAL(groupUpdateReceived(PalPacket)), this, SLOT(groupUpdateReceived(PalPacket)));
    connect(conn, SIGNAL(ownSubProfileReceived(PalPacket)), this, SLOT(ownSubProfileReceived(PalPacket)));
    connect(conn, SIGNAL(subProfileQueryResultReceived(PalPacket)), this, SLOT(subProfileQueryResultReceived(PalPacket)));
    connect(conn, SIGNAL(subProfileReceived(PalPacket)), this, SLOT(subProfileReceived(PalPacket)));
    connect(conn, SIGNAL(responseReceived(PalPacket)), this, SLOT(responseReceived(PalPacket)));
    connect(conn, SIGNAL(avatarReceived(PalPacket)), this, SLOT(avatarReceived(PalPacket)));
    connect(conn, SIGNAL(urlReceived(PalPacket)), this, SLOT(urlReceived(PalPacket)));
    connect(conn, SIGNAL(unknownPacketReceived(PalPacket)), this, SLOT(unknownPacketReceived(PalPacket)));
}


