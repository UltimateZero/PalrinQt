#ifndef JSON_H
#define JSON_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include "palrinqt_global.h"

class PALRINQTSHARED_EXPORT Json : public QObject
{
    Q_OBJECT
public:
    Json();
    QString encode(const QMap<QString,QVariant> &map);
    QMap<QString, QVariant> decode(const QString &jsonStr);

signals:

public slots:

private:
    QScriptValue encodeInner(const QMap<QString,QVariant> &map, QScriptEngine* engine);
    QMap<QString, QVariant> decodeInner(QScriptValue object);
    QList<QVariant> decodeInnerToList(QScriptValue arrayValue);

};

#endif // JSON_H
