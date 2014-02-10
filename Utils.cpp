#include "Utils.h"
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
#include <QDateTime>
#include <QUrl>

Utils::Utils(QObject *parent) :
    QObject(parent)
{
    setConstants();
}

QByteArray Utils::gender(const QByteArray &sex, bool reverse)
{
    if(reverse)
        return GENDER.key(sex, sex);
    return GENDER.value(sex, sex);
}

QByteArray Utils::lookingFor(const QByteArray &after, bool reverse)
{
    if(reverse)
        return LOOKING_FOR.key(after, after);
    return LOOKING_FOR.value(after, after);
}

QByteArray Utils::capabilities(const QByteArray &caps, bool reverse)
{
    if(reverse)
        return CAPABILITIES.key(caps, caps);
    return CAPABILITIES.value(caps, caps);
}

QByteArray Utils::relationshipStatus(const QByteArray &relstatus, bool reverse)
{
    if(reverse)
        return RELATIONSHIP_STATUS.key(relstatus, relstatus);
    return RELATIONSHIP_STATUS.value(relstatus, relstatus);
}

QByteArray Utils::deviceType(const QByteArray &device_type, bool reverse)
{
    if(reverse)
        return DEVICE_TYPES.key(device_type, device_type);
    return DEVICE_TYPES.value(device_type, device_type);
}

QByteArray Utils::onlineStatus(const QByteArray &online_status, bool reverse)
{
    if(reverse)
        return ONLINE_STATUSES.key(online_status, online_status);
    return ONLINE_STATUSES.value(online_status, online_status);
}

QByteArray Utils::language(const QByteArray &lang, bool reverse)
{
    if(reverse)
        return LANGUAGES.key(lang, lang);
    return LANGUAGES.value(lang, lang);
}

QByteArray Utils::country(const QByteArray &country, bool reverse)
{
    if(reverse)
        return COUNTRIES.key(country, country);
    return COUNTRIES.value(country, country);
}

int Utils::level(const QByteArray &rep)
{
    return repToLevel(rep.toInt());
}

QStringList Utils::privileges(const QByteArray &priv)
{
    return getPrivileges(priv.toInt());
}

QByteArray Utils::chatType(const QByteArray &type, bool reverse)
{
    if(reverse)
        return MSG_TARGET_TYPES.key(type, type);
    return MSG_TARGET_TYPES.value(type, type);
}

QByteArray Utils::groupAction(const QByteArray &action, bool reverse)
{
    if(reverse)
        return GROUP_ACTIONS.key(action, action);
    return GROUP_ACTIONS.value(action, action);
}

QByteArray Utils::convertToJpeg(const QByteArray &data)
{
    QBuffer imgbuffer;
    imgbuffer.open(QBuffer::ReadWrite);
    imgbuffer.write(data);
    imgbuffer.seek(0);

    QByteArray imageFormat = QImageReader::imageFormat(&imgbuffer);
    if(imageFormat == "jpeg")
        return data;
    else
    {
        QImage img;
        img.loadFromData(data);
        imgbuffer.reset();
        img.save(&imgbuffer, "JPG");
        imgbuffer.seek(0);
        QByteArray new_data = imgbuffer.readAll();
        imgbuffer.close();
        return new_data;
    }

}

QList<QByteArray> Utils::getChunks(const QByteArray &data, int chunkSize)
{
    QList<QByteArray> output;
    int dataCount = 0;
    for(int i = 0; i < data.length(); i+=chunkSize)
    {
        output.append(data.mid(i, chunkSize));
        dataCount += data.mid(i, chunkSize).length();
    }

    if(dataCount != data.length())
    {
        output.append(data.mid(dataCount));
    }

    return output;
}

float Utils::toPercent(int part, int full)
{
    if(full == 0)
        return -1;
    return (part*100)/full;
}






int Utils::repToLevel(int rep)
{
    for(int i = 0; i < LEVELS.values().length(); i++)
    {
        int current_rep = LEVELS.values().at(i);

        int prev_rep;
        int next_rep;

        if(i == 0)
        {
            next_rep = LEVELS.values().at(i+1);
            if(current_rep <= rep && rep < next_rep)
                return LEVELS.key(current_rep);

        }

        else if(i == LEVELS.values().length()-1)
        {
            prev_rep = LEVELS.values().at(i-1);
            if(prev_rep < rep && rep <= current_rep)
                return LEVELS.key(current_rep);
        }

        else
        {
            prev_rep = LEVELS.values().at(i-1);
            next_rep = LEVELS.values().at(i+1);
            if(prev_rep < rep && rep <= current_rep || current_rep <= rep && rep < next_rep)
                return LEVELS.key(current_rep);
        }
    }

    return -1;
}

QStringList Utils::getPrivileges(int priv)
{
    QStringList privs;
    bool isStaff = priv & 0x1000;
    bool isVip = priv & 0x200000;

    if(isStaff)
        privs << "Staff";
    else if(isVip)
        privs << "VIP";
    else if( (!isStaff && !isVip) || ( (priv & 0x40000) == 0) )
    {
        if(priv & 0x10000000)
            privs << "Agent";
        else
        {
            if(priv & 0x100000)
                privs << "Premium";
            else
                privs << "Normal";
        }
    }
    else
    {
        privs << "Pest";
    }

    return privs;
}

void Utils::setConstants()
{
    GENDER["0"] = "";
    GENDER["1"] = "Male";
    GENDER["2"] = "Female";
    //
    LOOKING_FOR["0"] = "";
    LOOKING_FOR["1"] = "Friendship";
    LOOKING_FOR["2"] = "Dating";
    LOOKING_FOR["3"] = "Friendship and Dating";
    LOOKING_FOR["4"] = "Relationship";
    LOOKING_FOR["5"] = "Relationship and Friendship";
    LOOKING_FOR["6"] = "Relationship and Dating";
    LOOKING_FOR["7"] = "Relationship, Friendship and Dating";
    LOOKING_FOR["8"] = "Networking";
    LOOKING_FOR["9"] = "Networking and Friendship";
    LOOKING_FOR["10"] = "Networking and Dating";
    LOOKING_FOR["11"] = "Networking, Dating and Friendship";
    LOOKING_FOR["12"] = "Relationship and Networking";
    LOOKING_FOR["13"] = "Relationship, Friendship and Networking";
    LOOKING_FOR["14"] = "Relationship, Dating and Networking";
    LOOKING_FOR["15"] = "Friendship, Dating, Relationship and Networking";
    //
    CAPABILITIES["0"] = "User";
    CAPABILITIES["1"] = "Admin";
    CAPABILITIES["2"] = "Mod";
    CAPABILITIES["3"] = "Silenced"; //Not sure yet
    CAPABILITIES["4"] = "Banned";
    //
    RELATIONSHIP_STATUS["0"] = "";
    RELATIONSHIP_STATUS["1"] = "Single";
    RELATIONSHIP_STATUS["2"] = "In a relationship";
    RELATIONSHIP_STATUS["3"] = "Engaged";
    RELATIONSHIP_STATUS["4"] = "Married";
    RELATIONSHIP_STATUS["5"] = "It's complicated";
    RELATIONSHIP_STATUS["6"] = "In an open relationship";
    //
    DEVICE_TYPES["0"] = "";
    DEVICE_TYPES["1"] = "Bot";
    DEVICE_TYPES["2"] = "PC";
    DEVICE_TYPES["3"] = "Symbian";
    DEVICE_TYPES["4"] = "Mac";
    DEVICE_TYPES["5"] = "iPhone";
    DEVICE_TYPES["6"] = "iPad";
    DEVICE_TYPES["7"] = "Android";
    //
    ONLINE_STATUSES["0"] = "Offline";
    ONLINE_STATUSES["1"] = "Online";
    ONLINE_STATUSES["2"] = "Away";
    ONLINE_STATUSES["5"] = "Busy";
    ONLINE_STATUSES["17"] = "Connecting";
    //
    MSG_TARGET_TYPES["0"] = "CHAT";
    MSG_TARGET_TYPES["1"] = "GROUPCHAT";
    //
    GROUP_ACTIONS["0"] = "RESET";
    GROUP_ACTIONS["1"] = "ADMIN";
    GROUP_ACTIONS["2"] = "MOD";
    GROUP_ACTIONS["4"] = "BAN";
    GROUP_ACTIONS["8"] = "SILENCE";
    GROUP_ACTIONS["16"] = "KICK";
    //
    LEVELS[0] = 0;
    LEVELS[1] = 100;
    LEVELS[2] = 200;
    LEVELS[3] = 400;
    LEVELS[4] = 800;
    LEVELS[5] = 1600;
    LEVELS[6] = 3200;
    LEVELS[7] = 6400;
    LEVELS[8] = 12800;
    LEVELS[9] = 25600;
    LEVELS[10] = 51200;
    LEVELS[11] = 102400;
    LEVELS[12] = 204800;
    LEVELS[13] = 409600;
    LEVELS[14] = 819200;
    LEVELS[15] = 1638400;
    //
    LANGUAGES["0"] = "";
    QFile languagesfile(":/text/resources/languagescodes.txt");
    if(languagesfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QStringList lines = QString(languagesfile.readAll()).split('\n', QString::SkipEmptyParts);
        foreach(QString line, lines)
            LANGUAGES[line.split('#')[0].toLower().toUtf8()] = line.split('#')[1].toUtf8();
        languagesfile.close();
    }
    //
    COUNTRIES["0"] = "";
    QFile countriesfile(":/text/resources/countrycodes.txt");
    if(countriesfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QStringList lines = QString(countriesfile.readAll()).split('\n', QString::SkipEmptyParts);
        foreach(QString line, lines)
            COUNTRIES[line.split('#')[0].toLower().toUtf8()] = line.split('#')[1].toUtf8();
        countriesfile.close();
    }
    //

}
