#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "palrinqt_global.h"

class PALRINQTSHARED_EXPORT Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = 0);

    QByteArray gender(const QByteArray &sex, bool reverse = false);
    QByteArray lookingFor(const QByteArray &after, bool reverse = false);
    QByteArray capabilities(const QByteArray &caps, bool reverse = false);
    QByteArray relationshipStatus(const QByteArray &relstatus, bool reverse = false);
    QByteArray deviceType(const QByteArray &device_type, bool reverse = false);
    QByteArray onlineStatus(const QByteArray &online_status, bool reverse = false);
    QByteArray language(const QByteArray &lang, bool reverse = false);
    QByteArray country(const QByteArray &country, bool reverse = false);
    int level(const QByteArray &rep);
    QStringList privileges(const QByteArray &priv);
    QByteArray chatType(const QByteArray &type, bool reverse = false);
    QByteArray groupAction(const QByteArray &action, bool reverse = false);

    static QByteArray convertToJpeg(const QByteArray &data);
    static QList<QByteArray> getChunks(const QByteArray &data, int chunkSize);

    static float toPercent(int part, int full);

signals:

public slots:


private:
    QMap<QByteArray, QByteArray> GENDER;
    QMap<QByteArray, QByteArray> LOOKING_FOR;
    QMap<QByteArray, QByteArray> CAPABILITIES;
    QMap<QByteArray, QByteArray> RELATIONSHIP_STATUS;
    QMap<QByteArray, QByteArray> DEVICE_TYPES;
    QMap<QByteArray, QByteArray> ONLINE_STATUSES;
    QMap<QByteArray, QByteArray> LANGUAGES;
    QMap<QByteArray, QByteArray> MSG_TARGET_TYPES;
    QMap<QByteArray, QByteArray> GROUP_ACTIONS;
    QMap<QByteArray, QByteArray> COUNTRIES;
    QMap<int, int> LEVELS;


    void setConstants();
    int repToLevel(int rep);
    QStringList getPrivileges(int priv);

};

#endif // UTILS_H
