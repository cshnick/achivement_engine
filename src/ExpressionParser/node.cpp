#include <QtCore>

#include "ExpressionParser.h"

Node::Node(Type type, const QString &str, const QVariant &res)
{
    this->type = type;
    this->str = str;
    this->res = res;
    parent = 0;
}

Node::~Node()
{
    qDeleteAll(children);
}
