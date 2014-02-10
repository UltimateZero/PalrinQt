#ifndef DATAMAPPARSER_H
#define DATAMAPPARSER_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include "palrinqt_global.h"

class PALRINQTSHARED_EXPORT DataMapParser : public QObject
{
    Q_OBJECT
public:
     DataMapParser(QObject *parent = 0);

     static QHash<QString, QVariant> getDataMap(const QByteArray &data);



signals:

public slots:

private slots:

private:
     static int readTilNull(int lastStart, const QByteArray &data);

     static QHash<QString, QVariant> parseDataMap(const QByteArray &data);

     static QHash<QByteArray, QByteArray> parseData(const QByteArray &data);

};

#endif // DATAMAPPARSER_H
