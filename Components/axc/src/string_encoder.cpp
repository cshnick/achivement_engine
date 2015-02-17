#include "string_encoder.h"
#include "QtCore"

String_encoder::String_encoder(QObject *parent) : QObject(parent)
{

}

String_encoder::~String_encoder()
{

}

QString String_encoder::fromUtf8(const QByteArray &arr)
{
    qDebug() << "Array";
    qDebug() << QString::fromUtf8(arr);
    return QString::fromUtf8(arr);
}
