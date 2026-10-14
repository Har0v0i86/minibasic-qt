#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include "expression.h"
#include "statement.h"

class Parser {
public:
    // 将一行代码解析为一个 Statement 指针
    Parser();
    Statement* parseStatement(QString line);

private:
    // 递归下降解析表达式的核心函数
    Expression* parseExpression(QStringList& tokens);
    Expression* parseTerm(QStringList& tokens);
    Expression* parseExponent(QStringList& tokens);
    Expression* parseFactor(QStringList& tokens);
    QStringList tokenize(QString line);
};

#endif // PARSER_H
