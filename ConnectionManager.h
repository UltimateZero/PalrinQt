#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include "PalPacket.h"
#include "PacketBuilder.h"

#include "palrinqt_global.h"

class PALRINQTSHARED_EXPORT ConnectionManager : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionManager(QObject *parent = 0);
    ~ConnectionManager();

    void setPrintCmds(bool print);

    void startConnection(const QByteArray &email, const QByteArray &password, int online_status = 1);
    void disconnectFromHost();
    void sendPing();

    int updateNickname(const QByteArray &nickname);
    int updateStatus(const QByteArray &status);
    int updateOnlineStatus(const QByteArray &online_status);
    int updateDetails(const QMap<QByteArray, QByteArray> &details);

    int sendMessage(int target_type, const QByteArray &target_id, QByteArray content_type, QByteArray data);

    int joinGroup(const QByteArray &group_name, const QByteArray &group_pw);
    int leaveGroup(const QByteArray &group_id);
    int sendGroupAction(const QByteArray &group_id, const QByteArray &target_id, int action);
    int getGroupInfo(QByteArray group, bool isName);
    int getGroupStats(const QByteArray &group_id);
    int searchGroups(QByteArray group_name, int offset, int max_results = 10);

    int acceptContact(const QByteArray &contact_id);
    int rejectContact(const QByteArray &contact_id);
    int addContact(const QByteArray &user, bool isEmail, const QByteArray &msg);
    int removeContact(const QByteArray &contact_id);
    int searchContacts(QByteArray nick, int max_results = 10);


    int getContactProfile(const QByteArray &contact_id);
    int getAvatar(const QByteArray &id, int size, bool isGroup);

    int sendCustomPacket(PalPacket packet);



public:


signals:
    void connected();
    void disconnected(bool error, QByteArray reason);
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



private slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketStateChanged(QAbstractSocket::SocketState);
    void readSocket();
    void writeSocket(const QByteArray &data);
    int sendPacket(PalPacket packet);



private:
    void handlePacket(PalPacket packet);
    void sendLOGON();
    void sendBye();
    void sendAUTH(PalPacket inpacket);
    int sendMessageBytes(const QByteArray &buffer, const QByteArray &content_type, int total_length, int total_chunks, int chunk_number, int correlation_id, int target_type, const QByteArray &target_id);
    QByteArray handleAUTH(PalPacket inpacket);




private:
    QTcpSocket *socket;
    PacketBuilder *builder;

    QByteArray DEFAULT_IP;
    int DEFAULT_PORT;
    QByteArray IP_ADDRESS;
    int PORT;


    QList<QByteArray> KNOWN_COMMANDS;
    QList<QByteArray> noMesgId;


    QByteArray ownEmail;
    QByteArray ownPassword;
    int ownOnlineStatus;


    //variables
    bool loggedIn;
    bool incorrectLogin;
    bool printCmds;
    int redirectCount;
    int ps;
    int mesgId;
    QByteArray lastDisconnectionReason;
    QByteArray lastPacket;
    //


    void setConstants();


};

#endif // CONNECTIONMANAGER_H
