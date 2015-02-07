#include <QtCore>

#include "ExpressionParser.h"
#include "node.h"

Node *ExpressionParser::parse(const QString &expr)
{
    in = expr;
    in.replace(" ", "");
    pos = 0;

    Node *node = new Node(Node::Root);
    addChild(node, parseOrExpression());
    return node;
}

Node *ExpressionParser::parseOrExpression()
{
    Node *childNode = parseAndExpression();
    if (matchToken("||")) {
        Node *node = new Node(Node::OrExpression);
        addChild(node, childNode);
        while (matchToken("||")) {
            addToken(node, "||", Node::Operator);
            addChild(node, parseAndExpression());
        }
        return node;
    } else {
        return childNode;
    }
}

Node *ExpressionParser::parseAndExpression()
{
    Node *childNode = parseCompExpression();
    if (matchToken("&&")) {
        Node *node = new Node(Node::AndExpression);
        addChild(node, childNode);
        while (matchToken("&&")) {
            addToken(node, "&&", Node::Operator);
            addChild(node, parseCompExpression());
        }
        return node;
    } else {
        return childNode;
    }
}

Node *ExpressionParser::parseCompExpression()
{
    Node *childNode = parseAddExpression();
    QString tok;
    if (matchTokens("<=|>=|<|>|!=|==", tok)) {
        Node *node = new Node(Node::CompExpression);
        addChild(node, childNode);
        while (matchTokens("<=|>=|<|>|!=|==", tok)) {
            addToken(node, tok, Node::Operator);
            addChild(node, parseAddExpression());
        }
        return node;
    } else {
        return childNode;
    }
}

Node *ExpressionParser::parseAddExpression() {
    Node *childNode = parseMulExpression();
    QString tok;
    if (matchTokens("+|-", tok)) {
        Node *node = new Node(Node::AddExpression);
        addChild(node, childNode);
        while (matchTokens("+|-", tok)) {
            addToken(node, tok, Node::Operator);
            addChild(node, parseMulExpression());
        }
        return node;
    } else {
        return childNode;
    }
}
Node *ExpressionParser::parseMulExpression() {
    Node *childNode = parseNotExpression();
    QString tok;
    if (matchTokens("*|/|%", tok)) {
        Node *node = new Node(Node::MulExpression);
        addChild(node, childNode);
        while (matchTokens("*|/|%", tok)) {
            addToken(node, tok, Node::Operator);
            addChild(node, parseNotExpression());
        }
        return node;
    } else {
        return childNode;
    }
}

Node *ExpressionParser::parseNotExpression()
{
    if (matchToken("!")) {
        Node *node = new Node(Node::NotExpression);
        while (matchToken("!"))
            addToken(node, "!", Node::Operator);
        addChild(node, parseAtom());
        return node;
    } else {
        return parseAtom();
    }
}

Node *ExpressionParser::parseAtom()
{
    if (matchToken("(")) {
        Node *node = new Node(Node::Atom);
        addToken(node, "(", Node::Punctuator);
        addChild(node, parseOrExpression());
        addToken(node, ")", Node::Punctuator);
        return node;
    } else {
        return parseIdentifier();
    }
}

//Sql expresssion
bool ExpressionParser::matchSql() {
	int startPos = pos;
	QString sqmatch  = in.mid(pos, 5);
	if (!sqmatch.compare("$sql{")) {
		pos += 5;
		return true;
	}
	return false;
}

Node *ExpressionParser::parseIdentifier()
{
    int startPos = pos;

    auto ndtype = Node::Identifier;
    QString str;
    QVariant res;
    //Variables start from "%"
    if (in[pos] == '$') {
    	if (in[pos + 1].isNumber()) {
    		++pos;
    		while (pos < in.length() && (in[pos].isNumber())) {
    			++pos;
    		}
    		ndtype = Node::AchCount;
    	} else if (in[pos + 1].isLetter()) {
    		++pos;
    		while (pos < in.length() && (in[pos].isLetterOrNumber())) {
    			++pos;
    		}
    		ndtype = Node::Identifier;
    	} else if (matchSql()) {
    		while (pos < in.length() && in[pos] != '}') {
    			++pos;
    		}
    		++pos; //include '}'
    		ndtype = Node::SqlExpression;
    	}
    	str = in.mid(startPos, pos - startPos);
    	res = str;

    } else if (in[pos].isNumber()) {
    	while (pos < in.length() && in[pos].isNumber()) {
    		++pos;
        }
        str = in.mid(startPos, pos - startPos);
        res = str.toInt();
        ndtype = Node::Number;
    }

    if (pos > startPos) {
        return new Node(ndtype, str, res);
    } else {
        return 0;
    }
}

void ExpressionParser::addChild(Node *parent, Node *child)
{
    if (child) {
        parent->children += child;
        parent->str += child->str;
        child->parent = parent;
    }
}

void ExpressionParser::addToken(Node *parent, const QString &str,
                             Node::Type type)
{
    if (in.mid(pos, str.length()) == str) {
        addChild(parent, new Node(type, str));
        pos += str.length();
    }
}

bool ExpressionParser::matchToken(const QString &str) const
{
    return in.mid(pos, str.length()) == str;
}

bool ExpressionParser::matchTokens(const QString &pattern, QString &tok) const
{
    QStringList tokens = pattern.split("|", QString::SkipEmptyParts);
    Q_FOREACH(QString ct, tokens) {
        if (matchToken(ct)) {
            tok = ct;
            return true;
        }
    }

    return false;
}
