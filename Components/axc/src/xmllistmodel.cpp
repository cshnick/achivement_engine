#include "xmllistmodel.h"
#include <QtCore>
#include <QtXml>

#include "Conventions.h"

static const QStringList s_fields(QString(QString(  AE::f_id) + "," +
													AE::f_name + "," +
													AE::f_description + "," +
													AE::f_condition
												).split(','));

static const QString filePath = AE::g_achivements_path;
static const QString fileName = AE::g_achivementsFileName;

class XMLListModelPrivate {
    friend class XMLListModel;
public:
    XMLListModelPrivate(XMLListModel *p_q) : q(p_q) {}

    int rowCount(const QModelIndex &parent) const {Q_UNUSED(parent); return m_elements.count();}
    QVariant data(const QModelIndex &index, int role) const {
        int rw = index.row();
        if (rw < 0 || rw > m_elements.count()) {
            return QVariant();
        }
        QVariantMap dta = m_elements.at(index.row());
//        if (role == s_fields.count() + Qt::UserRole + 1) {
//            return dta;
//        }
        return dta.value(s_fields.at(role - Qt::UserRole - 1));
    }

    QHash<int, QByteArray> roleNames() const {
        QHash<int, QByteArray> roles;
        for (int i = 0;i < s_fields.count(); i++) {
            roles[i + Qt::UserRole + 1] = s_fields.at(i).toUtf8();
        }
        roles[s_fields.count() + Qt::UserRole + 1] = "Data";
        return roles;
    }

    QStringList fields() const {
        return s_fields;
    }

    void append(const QVariantMap &p_data) {
        q->beginInsertRows(QModelIndex(), m_elements.count(), m_elements.count());
        m_elements.append(p_data);
        q->endInsertRows();
    }

    void insert(int index, const QVariantMap &p_data) {
        q->beginInsertRows(QModelIndex(), index, index);
        m_elements.insert(index, p_data);
        q->endInsertRows();
    }
    void update(int index, const QVariantMap &p_data) {
        if (index < 0 || index > m_elements.count()) {
            toXml();
            return;
        }
        QVariantMap &mp = m_elements[index];
        mp = p_data;
        toXml();
    }
    void remove(int index) {
        q->beginRemoveRows(QModelIndex(), index, index);
        m_elements.removeAt(index);
        q->endRemoveRows();
    }
    QVariant dict(int index) const {
        if (index < 0 || index > m_elements.count()) {
            return QVariantMap();
        }
        return m_elements.at(index);
    }
    int getId() const {
        return ++m_lastId;
    }
    bool toXml() {
        QFile file(filePath + "/" + fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Can't open for writing" << filePath + "/" + fileName;
            return false;
        }
        QXmlStreamWriter writer(&file);
        writer.writeStartDocument();
        writer.writeStartElement("root");
        for (int i = 0; i < m_elements.count(); i++) {
            QVariantMap element = m_elements.at(i);
            writer.writeStartElement(AE::tag_element);
            for (auto j = element.begin(); j != element.end() ; j++) {
                writer.writeTextElement(j.key(), j.value().toString());
            }
            writer.writeEndElement();
        }
        writer.writeTextElement(AE::tag_lastId, QString::number(m_lastId));
        writer.writeEndElement();
        writer.writeEndDocument();
        file.close();

        return true;
    }
    bool fromXml() {
        QDomDocument doc;
        QFile file(filePath + "/" + fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Can't open for reading" << filePath + "/" + fileName;
            return false;
        }
        QString err_string;
        int err_line;
        int err_column;
        if (!doc.setContent(&file, false, &err_string, &err_line, &err_column)) {
            qDebug() << "Can't set content for" << filePath + "/" + fileName;
            file.close();
            return false;
        }
        QDomElement element = doc.firstChildElement().firstChildElement(AE::tag_element);
        while (!element.isNull()) {
            QVariantMap dta;
            QDomElement elAttr = element.firstChildElement();
            while (!elAttr.isNull()) {
                //type conversion

                QVariant value;
                if (elAttr.tagName() == AE::f_id) {
                    int inttext = elAttr.text().toInt();
                    value = inttext;
                    m_lastId = qMax(inttext, m_lastId);
                } else {
                    value = elAttr.text();
                }
                dta[elAttr.tagName()] = value;
                elAttr = elAttr.nextSiblingElement();
            }
            append(dta);
            element = element.nextSiblingElement(AE::tag_element);
        }
        //Store last id to preserve unuque indecies
        QDomElement lid = doc.firstChildElement().firstChildElement(AE::tag_lastId);
        if (!lid.isNull()) {
            m_lastId = qMax(m_lastId, lid.text().toInt());
        }
        file.close();
        return true;
    }
    void addTest() {
        QVariantMap data1;
        data1[AE::f_id] = getId();
        data1[AE::f_name] = "Name1";
        data1[AE::f_description] = "Description 1";
        data1[AE::f_condition] = "Condition 1";

        append(data1);

        QVariantMap data2;
        data2[AE::f_id] = getId();
        data2[AE::f_name] = "Name2";
        data2[AE::f_description] = "Description 2";
        data2[AE::f_condition] = "Condition 2";

        append(data2);

        QVariantMap data3;
        data3[AE::f_id] = getId();
        data3[AE::f_name] = "Name3";
        data3[AE::f_description] = "Description 3";
        data3[AE::f_condition] = "Condition 3";

        append(data3);
    }

private:
    XMLListModel *q;
    mutable int m_lastId = 0;
    QList<QVariantMap> m_elements;
};

XMLListModel::XMLListModel(QObject *parent) :
    QAbstractListModel(parent)
  , p(new XMLListModelPrivate(this))
{
}
XMLListModel::~XMLListModel()
{
    delete p;
}

int XMLListModel::rowCount(const QModelIndex &parent) const
{
    return p->rowCount(parent);
}

QVariant XMLListModel::data(const QModelIndex &index, int role) const
{
    return p->data(index, role);
}

QStringList XMLListModel::fields() const
{
    return p->fields();
}

void XMLListModel::append(const QVariantMap &p_data)
{
    p->append(p_data);
}

void XMLListModel::insert(int index, const QVariantMap &p_data)
{
    p->insert(index, p_data);
}
void XMLListModel::update(int index, const QVariantMap &p_data)
{
    p->update(index, p_data);
    Q_EMIT dataChanged(this->index(index, 0), this->index(index, 0));
}
void XMLListModel::remove(int index)
{
    p->remove(index);
}
QVariant XMLListModel::dict(int index) const
{
    return p->dict(index);
}
int XMLListModel::getId() const
{
    return p->getId();
}
int XMLListModel::count() const
{
    return p->m_elements.count();
}
bool XMLListModel::fromXml()
{
    return p->fromXml();
}
void XMLListModel::addTest()
{
    p->addTest();
}
QHash<int, QByteArray> XMLListModel::roleNames() const {
    return p->roleNames();
}
void XMLListModel::emitDataChanged(const QModelIndex &start, const QModelIndex &end)
{
    Q_EMIT dataChanged(start, end);
}


