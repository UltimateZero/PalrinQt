#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QObject>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/cryptlib.h>
#include <cryptopp/salsa.h>
#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <cryptopp/zlib.h>
#include <cryptopp/osrng.h>
#include <string.h>

#include "palrinqt_global.h"

using namespace CryptoPP;
using namespace std;

class PALRINQTSHARED_EXPORT CryptoManager : public QObject
{
    Q_OBJECT
public:
    explicit CryptoManager(QObject *parent = 0);

    static QByteArray salsa20(QByteArray IV, QByteArray KEY, QByteArray DATA);
    static QByteArray md5(QByteArray data);
    static QByteArray dbMD5(QByteArray one, QByteArray two);

    static string zlibUncompress(string data);




signals:

public slots:

};

#endif // CRYPTOMANAGER_H
