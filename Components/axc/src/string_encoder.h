#ifndef STRING_ENCODER_H
#define STRING_ENCODER_H

#include <QObject>
#include <QString>

class String_encoder : public QObject
{
    Q_OBJECT

public:
    explicit String_encoder(QObject *parent = 0);
    ~String_encoder();

    Q_INVOKABLE QString fromUtf8(const QByteArray &arr);
};

#endif // STRING_ENCODER_H
