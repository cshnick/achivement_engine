#ifndef XMLLISTMODEL_H
#define XMLLISTMODEL_H

#include <QAbstractListModel>

class XMLListModelPrivate;
class XMLListModel : public QAbstractListModel
{
    Q_OBJECT
    friend class XMLListModelPrivate;
public:

    explicit XMLListModel(QObject *parent = 0);
    ~XMLListModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

public:
    Q_INVOKABLE QStringList fields() const;
    Q_INVOKABLE void append(const QVariantMap &p_data);
    Q_INVOKABLE void insert(int index, const QVariantMap &p_data);
    Q_INVOKABLE void update(int index, const QVariantMap &p_data);
    Q_INVOKABLE QVariantMap remove(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariant dict(int index) const;
    Q_INVOKABLE int getId() const;
    Q_INVOKABLE int count() const;
    Q_INVOKABLE bool fromXml();
    Q_INVOKABLE bool fromXml(const QString &text);
    Q_INVOKABLE QString toXml();
    Q_INVOKABLE void addTest();



protected:
    QHash<int, QByteArray> roleNames() const;

private:
    XMLListModelPrivate *p;
    void emitDataChanged(const QModelIndex &start, const QModelIndex &end);
};

#endif // XMLLISTMODEL_H
