#ifndef BASECLIENT_H
#define BASECLIENT_H

#include <QObject>
#include "palrinqt_global.h"

#include "PalPacket.h"
#include "ConnectionManager.h"
#include "PalMessage.h"
#include "Utils.h"

#include <QMap>
#include <QTimer>

class PALRINQTSHARED_EXPORT BaseClient : public QObject
{
    Q_OBJECT
public:
    explicit BaseClient(QObject *parent = 0);
    ~BaseClient();

    void connectToHost(const QByteArray &email, const QByteArray &password);
    void connectToHost();
    void setEmail(const QByteArray &email);
    void setPassword(const QByteArray &password);
    void setAutoReconnected(bool autoreconnect);
    void setStoreData(bool storedata);

    //
    int sendMessage(const QString &id, QString body, int target_type = 0);
    int sendImage(const QString &id, QByteArray body, int target_type = 0);
    int sendAudio(const QString &id, QByteArray body, int target_type = 0);
    //
    int joinGroup(const QByteArray &group_name, QByteArray group_pw = QByteArray());
    int leaveGroup(const QByteArray &group_id);
    int kickUser(const QByteArray &group_id, const QByteArray &target_id);
    int silenceUser(const QByteArray &group_id, const QByteArray &target_id);
    int resetUser(const QByteArray &group_id, const QByteArray &target_id);
    int banUser(const QByteArray &group_id, const QByteArray &target_id);
    int modUser(const QByteArray &group_id, const QByteArray &target_id);
    int adminUser(const QByteArray &group_id, const QByteArray &target_id);
    //
    int addContact(const QByteArray &user, bool isEmail, QByteArray msg = QByteArray("I'd like to add you."));
    int removeContact(const QByteArray &contact_id);
    int acceptContact(const QByteArray &contact_id);
    int rejectContact(const QByteArray &contact_id);
    //
    int getContactProfile(const QByteArray &contact_id);
    int getAvatar(const QByteArray &id, int size = 640, bool isGroup = false);
    //
    int getGroupInfo(const QByteArray &group, bool isName = false);
    //
    QHash<QString, QVariant> getStoredContactData(const QString &contact_id);
    QHash<QString, QVariant> getStoredGroupData(const QString &group_id);
    QHash<QString, QVariant> getStoredSelfData();


signals:
    void connected();
    void disconnected(QByteArray reason);
    void messageReceived(PalMessage message);
    void contactDataReceived(QString contact_id, const QHash<QString, QVariant> &contact_data);
    void selfDataReceived(const QHash<QString, QVariant> &own_data);
    void contactAddRequestReceived(QString contact_id, const QHash<QString, QVariant> &contact_data);
    void groupDataReceived(QString group_id, const QHash<QString, QVariant> &group_data);
    void contactProfileReceived(QHash<QString, QVariant> data, QByteArray id);
    void avatarReceived(QByteArray avatar, int mesg_id);
    void urlDataReceived(QByteArray data, int mesg_id);
    void urlDataReceived(QMap<QString, QVariant> map, int mesg_id);
    void packetPartReceived(int correlation_id, int percentage);

private slots:
    void connectedToHost();
    void disconnectedFromHost(bool error, QByteArray reason);
    void reconnect();
    void messageReceived(PalPacket packet);
    void groupAdminReceived(PalPacket packet);
    void groupUpdateReceived(PalPacket packet);
    void ownSubProfileReceived(PalPacket packet);
    void subProfileQueryResultReceived(PalPacket packet);
    void subProfileReceived(PalPacket packet);
    void responseReceived(PalPacket packet);
    void avatarReceived(PalPacket packet);
    void urlReceived(PalPacket packet);
    void unknownPacketReceived(PalPacket packet);

private:
    ConnectionManager *conn;
    QTimer *reconnect_timer;
    Utils *utils;

    QMap<QString, QVariant> config;

    //
    QStringList contacts_list;
    QHash<QString, QVariant> self;
    QHash<QString, QHash<QString, QVariant> > contacts;
    QHash<QString, QHash<QString, QVariant> > groups;
    QHash<QString, QHash<QString, QVariant> > contact_adds;

    //
    QMap<int, QPair<int, QByteArray> > incompletePackets;



private:
    void updateContact(const QString &contact_id, const QHash<QString, QVariant> &contact_data);
    void updateGroup(const QString &group_id, const QHash<QString, QVariant> &group_data);
    void updateSelf(const QHash<QString, QVariant> &data);
    QList<QByteArray> parseContacts(const QHash<QString, QVariant> &all, bool force = false);
    QList<QByteArray> parseGroups(const QHash<QString, QVariant> &all);
    QList<QByteArray> parseAdds(const QHash<QString, QVariant> &all);

    bool isLastPacket(PalPacket *packet);

    void registerSignals();

};

#endif // BASECLIENT_H
