#include "ConnectionManager.h"
#include "CryptoManager.h"
#include "Utils.h"

#include <QDateTime>
#include <QUrl>


ConnectionManager::ConnectionManager(QObject *parent) :
    QObject(parent)
{
    setConstants();
    socket = new QTcpSocket(this);
    builder = new PacketBuilder(this);
    socket->setReadBufferSize(99999);
    IP_ADDRESS = DEFAULT_IP;
    PORT = DEFAULT_PORT;

    //
    ps = 0;
    mesgId = 0;
    redirectCount = 0;
    loggedIn = false;
    incorrectLogin = false;
    printCmds = true;
    //

    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));



}

ConnectionManager::~ConnectionManager()
{
    socket->close();
    delete socket;
    delete builder;
}

void ConnectionManager::setPrintCmds(bool print)
{
    printCmds = print;
}

void ConnectionManager::startConnection(const QByteArray &email, const QByteArray &password, int online_status)
{
    socket->setProperty("email", email);
    socket->setProperty("password", password);
    socket->setProperty("online_status", online_status);
    socket->disconnectFromHost();
    socket->connectToHost(IP_ADDRESS, PORT);
}


void ConnectionManager::disconnectFromHost()
{
    sendBye();
    socket->disconnectFromHost();
}


void ConnectionManager::socketConnected()
{
    qDebug() << "socket connected";
    sendLOGON();
}

void ConnectionManager::socketDisconnected()
{
    qDebug() << "socket disconnected";
    loggedIn = false;
    emit disconnected(incorrectLogin, lastDisconnectionReason);
    lastDisconnectionReason.clear();
}

void ConnectionManager::socketError(QAbstractSocket::SocketError error)
{
    qDebug() << "socket error " << error;
    socket->disconnectFromHost();
}

void ConnectionManager::socketStateChanged(QAbstractSocket::SocketState)
{
 //
}

void ConnectionManager::readSocket()
{

    QByteArray receivedData = lastPacket + socket->readAll();
    lastPacket.clear();
    if(receivedData.isEmpty())
        return;

//    qDebug() << "NEW DATA RECEIVED";


    PalPacket tempPacket(false);

    bool isValid = tempPacket.fromRawData(receivedData);
    if(!isValid)
    {
//        qDebug() << "Invalid/incomplete packet, calling waitForReadyRead()";
        while(true)
        {
            socket->waitForReadyRead(1000*3);
            receivedData += socket->readAll();
            isValid = tempPacket.fromRawData(receivedData);
            if(isValid)
                break;
        }
//        qDebug() << "Packet fixed";
    }


    QByteArray contentPayload = tempPacket.getCompressedPayload();
    int currentLength = contentPayload.size();
    int contentLength = tempPacket.hasHeader("CONTENT-LENGTH") ? tempPacket.getHeader("CONTENT-LENGTH").toInt() : 0;

    if(currentLength != contentLength)
    {
        qlonglong startTime = QDateTime::currentMSecsSinceEpoch()/1000;
        QByteArray bytes = socket->readAll();
        contentPayload.append(bytes); //append to temp payload
        currentLength += bytes.length();
        while(true)
        {
            if(currentLength == contentLength)
            {
//                qDebug() << "Length match! " << currentLength << contentLength;
                break;
            }

            else if(currentLength < contentLength)
            {
                socket->waitForReadyRead(1000*3);
                bytes = socket->readAll();
                contentPayload.append(bytes); //append to temp payload
                currentLength += bytes.length();

                if(QDateTime::currentMSecsSinceEpoch()/1000 - startTime > 15 && currentLength < contentLength)
                {
                    qDebug() << "Timeout. Skipping packet";
                    qDebug() << contentPayload.toHex();
                    break;
                }
            }

            else //If bigger, truncate
            {
                QByteArray extraBytes = contentPayload.mid(contentLength);
                lastPacket = extraBytes;
                contentPayload = contentPayload.left(contentLength);
                break;
            }


        }

    } //End check content length

    tempPacket.addPayload(contentPayload);
    bool isComplete = !(tempPacket.hasHeader("TOTAL-LENGTH") || tempPacket.hasHeader("CORRELATION-ID"));

    PalPacket packet(isComplete);
    packet.fromRawData(tempPacket.getRawPacket());
    handlePacket(packet);




    if(!lastPacket.isEmpty())
    {
//        qDebug() << "Found remaining data, recalling readSocket()";
        readSocket();
    }

}

void ConnectionManager::writeSocket(const QByteArray &data)
{
    socket->write(data);
    socket->waitForBytesWritten();
}

void ConnectionManager::handlePacket(PalPacket packet)
{
    QByteArray cmd = packet.getCommand();

    if(cmd == "AUTH")
    {
        qDebug() << "Authorizing...";
        sendAUTH(packet);
    }

    else if(cmd == "LOGON FAILED")
    {
        if(packet.getPayload().isEmpty())
        {
            incorrectLogin = true;
            qDebug() << "Incorrect Login";
            lastDisconnectionReason = "Email/Password are incorrect.";
        }
        else if(packet.getPayload().count('.') == 3)
        {
            IP_ADDRESS = packet.getPayload();
            qDebug() << "Reconnecting to new IP: " << IP_ADDRESS;
            redirectCount++;
            lastDisconnectionReason = "Redirected";
            socket->disconnectFromHost();
            socket->connectToHost(IP_ADDRESS, PORT);
        }
        else
        {
            qDebug() << "Failed to login, reason: " << packet.getPayload();
        }
    }

    else if(cmd == "P")
    {
        sendPing();
    }

    else if(cmd == "SUB PROFILE")
    {
        if(!loggedIn)
        {
            qDebug() << "Authorized";
            loggedIn = true;
            emit connected();
            emit ownSubProfileReceived(packet);
        }
        else
        {
            emit subProfileReceived(packet);
        }
    }

    else if(cmd == "SUB PROFILE QUERY RESULT")
    {
        emit subProfileQueryResultReceived(packet);
    }

    else if(cmd == "MESG")
    {
        emit messageReceived(packet);
    }

    else if(cmd == "GROUP ADMIN")
    {
        emit groupAdminReceived(packet);
    }

    else if(cmd == "GROUP UPDATE")
    {
        emit groupUpdateReceived(packet);
    }

    else if(cmd == "RESPONSE")
    {
        emit responseReceived(packet);
        handleMsgResponse(packet.mesgId(), packet.getPayload().toHex().toInt(0, 16));
    }

    else if(cmd == "AVATAR")
    {
        emit avatarReceived(packet);
    }

    else if(cmd == "URL")
    {
        emit urlReceived(packet);
    }

    else if(cmd == "GHOSTED")
    {
        lastDisconnectionReason = "Ghosted";
    }

    else if(cmd == "THROTTLE")
    {
        qDebug() << "THROTTLE FOR" << packet.getHeader("DURATION");
        //TODO: Add a handler? Or just leave it as it is?
    }

    else if(cmd == "BALANCE QUERY RESULT")
    {
        //emit this
    }

    else
    {
        emit unknownPacketReceived(packet);
    }

    if(cmd != "P" && printCmds == true)
        qDebug() << cmd;

}

int ConnectionManager::sendPacket(PalPacket packet)
{
    packet.addHeader("content-length", QByteArray::number(packet.getPayload().size()));
    if(!noMesgId.contains(packet.getCommand()))
    {
        mesgId++;
        packet.addHeader("mesg-id", QByteArray::number(mesgId));
    }
    if(packet.getCommand() == "P" || packet.getCommand() == "BYE")
    {
        ps++;
        packet.addHeader("ps", QByteArray::number(ps));
    }
    writeSocket(packet.getRawPacket());
    return mesgId;
}


int ConnectionManager::updateNickname(const QByteArray &nickname)
{
    PalPacket packet = builder->ContactDetailPacket();
    packet.addHeader("nickname", nickname);
    return sendPacket(packet);
}

int ConnectionManager::updateStatus(const QByteArray &status)
{
    PalPacket packet = builder->ContactDetailPacket();
    packet.addHeader("status", status);
    return sendPacket(packet);
}

int ConnectionManager::updateOnlineStatus(const QByteArray &online_status)
{
    PalPacket packet = builder->ContactDetailPacket();
    packet.addHeader("online-status", online_status);
    return sendPacket(packet);
}

int ConnectionManager::updateDetails(const QMap<QByteArray, QByteArray> &details)
{
    PalPacket packet = builder->ContactDetailPacket();
    foreach(QByteArray key, details.keys())
        packet.addHeader(key, details[key]);
    return sendPacket(packet);
}

int ConnectionManager::sendMessage(int target_type, const QByteArray &target_id, QByteArray content_type, QByteArray data)
{
    int correlation_id = -1;
    if(content_type.startsWith("image/"))
        data = Utils::convertToJpeg(data);
    QList<QByteArray> dataChunks = Utils::getChunks(data, 512);
    int chunk_number = 1;
    int currentMesgId = correlation_id;

    currentMesgId = sendMessageBytes(dataChunks.first(), content_type, data.size(), dataChunks.size(), chunk_number, correlation_id, target_type, target_id);
    correlation_id = currentMesgId;

    beingSent[correlation_id] = dataChunks;

    sentMsgs[currentMesgId]["correlationId"] = correlation_id;
    sentMsgs[currentMesgId]["contentType"] = content_type;
    sentMsgs[currentMesgId]["totalLength"] = data.size();
    sentMsgs[currentMesgId]["totalChunks"] = dataChunks.size();
    sentMsgs[currentMesgId]["targetType"] = target_type;
    sentMsgs[currentMesgId]["chunkNumber"] = chunk_number+1;
    sentMsgs[currentMesgId]["targetId"] = target_id;
    sentMsgs[currentMesgId]["isLast"] = chunk_number == dataChunks.size();

    return currentMesgId;
}


int ConnectionManager::joinGroup(const QByteArray &group_name, const QByteArray &group_pw)
{
    return sendPacket(builder->GroupSubscribePacket(group_name, group_pw));
}

int ConnectionManager::leaveGroup(const QByteArray &group_id)
{
    return sendPacket(builder->GroupUnsubPacket(group_id));
}

int ConnectionManager::sendGroupAction(const QByteArray &group_id, const QByteArray &target_id, int action)
{
    return sendPacket(builder->GroupAdminPacket(group_id, target_id, action));
}

int ConnectionManager::getGroupInfo(QByteArray group, bool isName)
{
    group = QUrl::toPercentEncoding(group);
    QByteArray action = builder->buildDataMap("action", "palringo_group_info");
    QByteArray parameters = builder->buildDataMap("parameters", (isName ? "name=" : "id=") + group );

    QByteArray payload = action+parameters;

    return sendPacket(builder->ProvifQueryPacket(payload));
}

int ConnectionManager::getGroupStats(const QByteArray &group_id)
{
    QByteArray payload = "http://www.palringo.com/c/api.php?action=palringo.group.stat.loadactive&group_id="+group_id;

    return sendPacket(builder->UrlPacket(payload));
}

int ConnectionManager::searchGroups(QByteArray group_name, int offset, int max_results)
{
    mesgId++;
    group_name = QUrl::toPercentEncoding(group_name);
    QByteArray action = builder->buildDataMap("action", "palringo_groups");
    QByteArray parameters = builder->buildDataMap("parameters", "offset=" + QByteArray::number(offset) + "&max_results=" + QByteArray::number(max_results) + (group_name.isEmpty() ? "" : "&name=" + group_name) );

    QByteArray payload = action+parameters;

    return sendPacket(builder->ProvifQueryPacket(payload));
}

int ConnectionManager::acceptContact(const QByteArray &contact_id)
{
    return sendPacket(builder->ContactAddResponsePacket(contact_id, true));
}

int ConnectionManager::rejectContact(const QByteArray &contact_id)
{
    return sendPacket(builder->ContactAddResponsePacket(contact_id, false));
}

int ConnectionManager::addContact(const QByteArray &user, bool isEmail, const QByteArray &msg)
{
    return sendPacket(builder->ContactAddPacket(user, msg, isEmail));
}

int ConnectionManager::removeContact(const QByteArray &contact_id)
{
    PalPacket packet = builder->ContactUpdatePacket(contact_id);
    packet.addHeader("remove", "1");

    return sendPacket(packet);
}

int ConnectionManager::searchContacts(QByteArray nick, int max_results)
{
    nick = QUrl::toPercentEncoding(nick);
    QByteArray action = builder->buildDataMap("action", "palringo_find_contact");
    QByteArray parameters = builder->buildDataMap("parameters", "max_results=" + QByteArray::number(max_results) + "&nickname=" + nick );
    //TODO: &first_name=first&last_name=last&age_range=25-29&gender=M&tags=test

    QByteArray payload = action+parameters;

    return sendPacket(builder->ProvifQueryPacket(payload));
}

int ConnectionManager::getContactProfile(const QByteArray &contact_id)
{
    mesgId++;
    QByteArray payload = builder->buildDataMap("Sub-Id", contact_id);

    return sendPacket(builder->SubProfileQueryPacket(payload));
}

int ConnectionManager::getAvatar(const QByteArray &id, int size, bool isGroup)
{
    return sendPacket(builder->AvatarPacket(id, size, isGroup));
}


int ConnectionManager::sendCustomPacket(PalPacket packet)
{
    return sendPacket(packet);
}


void ConnectionManager::sendPing()
{
    sendPacket(builder->PingPacket());
}

void ConnectionManager::sendLOGON()
{
    sendPacket(builder->LogonPacket(socket->property("email").toByteArray(), redirectCount));
}


void ConnectionManager::sendBye()
{
    sendPacket(builder->ByePacket());
}

void ConnectionManager::sendAUTH(PalPacket inpacket)
{
    sendPacket(builder->AuthPacket(handleAUTH(inpacket), socket->property("online_status").toInt()));
}

int ConnectionManager::sendMessageBytes(const QByteArray &buffer, const QByteArray &content_type, int total_length, int total_chunks, int chunk_number, int correlation_id, int target_type, const QByteArray &target_id)
{
    PalPacket packet = builder->MesgPacket(buffer, content_type, target_id, target_type);

    if(chunk_number == 1 && chunk_number != total_chunks) //Header
    {
        packet.addHeader("total-length", QByteArray::number(total_length));
    }

    if(chunk_number != total_chunks && chunk_number != 1) //Body
    {

        packet.addHeader("correlation-id", QByteArray::number(correlation_id)); //First mesg-id
    }

    if(chunk_number == total_chunks) //Footer
    {
        if(chunk_number != 1)
            packet.addHeader("correlation-id", QByteArray::number(correlation_id));
        packet.addHeader("last","1");
    }

    return sendPacket(packet);
}
QByteArray ConnectionManager::handleAUTH(PalPacket inpacket)
{
    QByteArray payload = inpacket.getPayload();
    QByteArray rnd = inpacket.getHeader("TIMESTAMP").remove(10, 1);

    QByteArray IV = payload.mid(16, 8);
    QByteArray authKey = CryptoManager::dbMD5(socket->property("password").toByteArray(), IV);
    QByteArray dataToEncrypt = payload.left(16) + rnd;

    QByteArray result = CryptoManager::salsa20(IV, authKey, dataToEncrypt);

    return result;
}

void ConnectionManager::handleMsgResponse(int id, int code)
{
    if(code == 4)
    {
        if(!sentMsgs.contains(id))
            return;
        if(sentMsgs[id]["isLast"].toBool())
        {
            beingSent.remove(sentMsgs[id]["correlationId"].toInt());
            sentMsgs.remove(id);
            return;
        }
        else
        {
            QByteArray contentType = sentMsgs[id]["contentType"].toByteArray();
            int totalLength = sentMsgs[id]["totalLength"].toInt();
            int totalChunks = sentMsgs[id]["totalChunks"].toInt();
            int correlationId = sentMsgs[id]["correlationId"].toInt();
            int targetType = sentMsgs[id]["targetType"].toInt();
            int chunkNumber = sentMsgs[id]["chunkNumber"].toInt();
            QByteArray targetId = sentMsgs[id]["targetId"].toByteArray();
            QByteArray nextBuffer = beingSent[correlationId][chunkNumber-1];

            if(chunkNumber % 5 == 0)
                delay();

            int newId = sendMessageBytes(nextBuffer, contentType, totalLength, totalChunks, chunkNumber, correlationId, targetType, targetId);

            sentMsgs[newId] = sentMsgs[id];
            sentMsgs.remove(id);
            sentMsgs[newId]["chunkNumber"] = chunkNumber+1;
            sentMsgs[newId]["isLast"] = chunkNumber == totalChunks;
        }



    }
    else
    {
        qDebug() << "Failed to send message" << id << code;
    }
}




void ConnectionManager::setConstants()
{
    DEFAULT_IP = "80.69.129.75";
    DEFAULT_PORT = 38535;
    //
    KNOWN_COMMANDS << "AUTH"
                  << "LOGON FAILED"
                  << "SUB PROFILE"
                  << "SUB PROFILE QUERY RESULT"
                  << "RESPONSE"
                  << "MESG"
                  << "BALANCE QUERY RESULT"
                  << "GROUP UPDATE"
                  << "GROUP ADMIN"
                  << "P"
                  << "AVATAR"
                  << "THROTTLE"
                  << "GHOSTED"
                  << "URL";
    //
    noMesgId << "LOGON"
             << "AUTH"
             << "P"
             << "BYE";

}
