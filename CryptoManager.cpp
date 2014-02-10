#include "CryptoManager.h"
#include <QCryptographicHash>
#include <QProcess>
#include <QDebug>
#include <QByteArray>
#include <QString>


CryptoManager::CryptoManager(QObject *parent) :
    QObject(parent)
{
}



string CryptoManager::zlibUncompress(string data)
{
    CryptoPP::ZlibDecompressor dec;

    string out;

    try {
        dec.Attach(new CryptoPP::StringSink(out));
        dec.Put((byte*)data.data(), data.size());
        dec.MessageEnd();
    } catch(...) {qDebug() << "Error decompressing string"; qDebug() << QByteArray(data.c_str(), data.length()).toHex();}

    return out;
}


QByteArray CryptoManager::salsa20(QByteArray IV, QByteArray KEY, QByteArray DATA)
{
    //pl("crypt-> encrypting to salsa20",1);

    //set up encryption
    Salsa20::Encryption enc;

    enc.SetKeyWithIV((byte*)KEY.constData(), KEY.length(), (byte*)IV.constData());

    //create and move data to buffer
    string ENC = "";
    StreamTransformationFilter stf( enc, new StringSink( ENC ) );
    stf.Put( (byte*)DATA.constData(), DATA.length() );
    stf.MessageEnd();

    //return buffer
    return QByteArray(ENC.c_str(), ENC.length());
}


QByteArray CryptoManager::md5(QByteArray data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

QByteArray CryptoManager::dbMD5(QByteArray one, QByteArray two)
{
    QByteArray ONE = md5(one);
    return md5(ONE+two);
}

