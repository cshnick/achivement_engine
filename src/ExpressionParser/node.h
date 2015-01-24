#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>
#include <QVariant>

class Node
{
public:
    enum Type { Root,
    			CompExpression,
                OrExpression,
                AndExpression,
                AddExpression,
                SubExpression,
                MulExpression,
                DivExpression,
                NotExpression,
                Atom,
                Identifier,
                Operator,
                Punctuator,
                Number
              };

    Node(Type type, const QString &str = "", const QVariant &res = QVariant());
    ~Node();

    Type type;
    QString str;
    QVariant res;
    Node *parent;
    QList<Node *> children;
};

#endif
