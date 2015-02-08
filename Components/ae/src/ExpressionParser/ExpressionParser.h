#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "node.h"

class ExpressionParser
{
public:
    Node *parse(const QString &expr);

private:
    Node *parseCompExpression();
    Node *parseOrExpression();
    Node *parseAndExpression();
    Node *parseAddExpression();
    Node *parseMulExpression();
    Node *parseNotExpression();
    Node *parseAtom();
    Node *parseIdentifier();
    void addChild(Node *parent, Node *child);
    void addToken(Node *parent, const QString &str, Node::Type type);
    bool matchToken(const QString &str) const;
    bool matchTokens(const QString &str, QString &tok) const;
    bool matchSql();
    void heuristic_trim();

    QString in;
    int pos;
};

#endif // EXPRESSIONPARSER_H
