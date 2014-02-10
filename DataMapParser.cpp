#include "DataMapParser.h"
#include <QDebug>

DataMapParser::DataMapParser(QObject *parent) :
    QObject(parent)
{
}

QHash<QString, QVariant> DataMapParser::getDataMap(const QByteArray &data)
{
    QHash<QString, QVariant> dataMap = parseDataMap(data);
//    qDebug() << dataMap;

    if(dataMap.contains("contacts"))
    {
        QHash<QString, QVariant> contacts = dataMap["contacts"].toHash();
        foreach(QString id, contacts.keys())
        {
            bool ok;
            id.toInt(&ok);
            if(!ok)
            {
                qDebug() << "FOUND MALFORMED ID";
                qDebug() << data.toHex();
                break;
            }
        }
    }

    return dataMap;
}

QHash<QString, QVariant> DataMapParser::parseDataMap(const QByteArray &data)
{
    QHash<QByteArray, QByteArray> initial = parseData(data);
    QHash<QString, QVariant> result;
//    qDebug() << "Initial: " << initial.keys();

    foreach(QByteArray key, initial.keys())
    {
//        qDebug() << key << ": " << initial[key];
        if(readTilNull(0, initial[key]) != -1) //Another map
        {
//            qDebug() << "Key" << key << " value contain null; parsing...";
            QHash<QString, QVariant> map = parseDataMap(initial[key]);
            result[key] = map;
//            qDebug() << "map:" << map;

        }
        else
        {
//            qDebug() << "Key" << key << " value is clean";
            result[key] = initial[key];
        }
    }


    return result;
}





QHash<QByteArray, QByteArray> DataMapParser::parseData(const QByteArray &data)
{
    int lastStart = 0;
    int nextNull = 0;

    QHash<QByteArray, QByteArray> output;

    while(true)
    {
        nextNull = readTilNull(lastStart, data);

//        qDebug() << "current nextNull:" << nextNull;
//        qDebug() << "current lastStart:" << lastStart;

        if(nextNull == -1)
        {
//            qDebug() << "Breaking out of the parse while loop";
            break;
        }

        QByteArray head;

        for(int i = lastStart; i < nextNull; i++)
            head.append(data.at(i));

        head = head.toLower();

        int BTR_length = 2;
        int BTR = data.mid(nextNull+1, BTR_length).toHex().toInt(0, 16);  //for the head value, 100% correct.

        int tailStart = nextNull+BTR_length+1;

        QByteArray tail = data.mid(tailStart, BTR);
        lastStart = tailStart+tail.length();

        if(output.contains(head))
        {
            output[head].append(tail);
        }
        else
            output[head] = tail;
    }

    return output;
}

int DataMapParser::readTilNull(int lastStart, const QByteArray &data)
{
    int newStart = -1;
    while(true)
    {
        if(lastStart >= data.size())
            return -1;

        if(data[lastStart] == '\0')
        {
            QByteArray temp;
            temp.append(data[lastStart]);
            return lastStart; //newStart
        }

        lastStart++;

    }

    return newStart;
}
