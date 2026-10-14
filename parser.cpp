#include "parser.h"

Parser::Parser() {} // 构造函数

Statement* Parser::parseStatement(QString line) {
    QStringList tokens = tokenize(line);
    if (tokens.isEmpty()) return nullptr;

    QString cmd = tokens.takeFirst().toUpper(); // 提取命令并转大写 (LET, PRINT...)

    if (cmd == "REM") {
        // 获取 REM 之后的所有内容作为注释
        int remIndex = line.indexOf("REM", 0, Qt::CaseInsensitive);
        QString commentStr = "";
        if (remIndex != -1) {
            // +3 是跳过 "REM" 三个字符
            commentStr = line.mid(remIndex + 3).trimmed();
        }
        return new RemStmt(commentStr.toStdString());
    }
    else if (cmd == "LET") {
        // 语法: LET var = expression
        // tokens[0] 是变量名, tokens[1] 是 "=", tokens[2...] 是表达式
        if (tokens.size() < 3 || tokens[1] != "=") {
            throw std::runtime_error("Invalid LET syntax");
        }
        QString varName = tokens[0];
        tokens.takeFirst(); // 消除变量名
        tokens.takeFirst(); // 消除 "="
        Expression* exp = parseExpression(tokens);
        return new LetStmt(varName.toStdString(), exp);
    }
    else if (cmd == "PRINT") {
        // 语法: PRINT expression
        Expression* exp = parseExpression(tokens);
        return new PrintStmt(exp);
    }
    else if (cmd == "INPUT") {
        // 语法: INPUT var
        if (tokens.isEmpty()) {
            throw std::runtime_error("Syntax Error: INPUT missing variable");
        }

        QString varName = tokens.first();
        return new InputStmt(varName.toStdString());
    }
    else if (cmd == "GOTO") {
        // 语法: GOTO line_number
        int lineNum = tokens.first().toInt();
        return new GotoStmt(lineNum);
    }
    else if (cmd == "IF") {
        // 语法: IF exp1 op exp2 THEN target

        // 1. 解析第一个表达式
        Expression* lhs = parseExpression(tokens);

        // 2. 提取操作符 (=, <, >)
        QString op = tokens.takeFirst();

        // 3. 解析第二个表达式
        Expression* rhs = parseExpression(tokens);

        // 4. 检查 THEN
        if (tokens.takeFirst().toUpper() != "THEN") {
            throw std::runtime_error("IF statement missing THEN");
        }

        // 5. 提取跳转行号
        int targetLine = tokens.first().toInt();

        return new IfStmt(lhs, op.toStdString(), rhs, targetLine);
    }
    else if (cmd == "END") {
        return new EndStmt();
    }

    throw std::runtime_error("Unknown command: " + cmd.toStdString());
}

QStringList Parser::tokenize(QString line)
{
    line.replace("("," ( ");
    line.replace(")"," ) ");
    line.replace("**", " POWER_OP ");
    line.replace("+"," + ");
    line.replace("-"," - ");
    line.replace("*"," * ");
    line.replace("/"," / ");
    line.replace("MOD"," MOD ");
    line.replace("="," = ");
    line.replace("<"," < ");
    line.replace(">"," > ");
    line.replace("POWER_OP", " ** ");

    return line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}


Expression* Parser::parseExpression(QStringList& tokens) {
    // 1. 先解析左边的项 (Term)
    Expression* lhs = parseTerm(tokens);

    // 2. 看看后面有没有 + 或 -
    while (!tokens.isEmpty() && (tokens.first() == "+" || tokens.first() == "-")) {
        QString op = tokens.takeFirst(); // 取出符号
        Expression* rhs = parseTerm(tokens);   // 解析右边的项
        // 组合成复合表达式树节点
        lhs = new CompoundExp(lhs, op.toStdString(), rhs);
    }
    return lhs;
}

Expression* Parser::parseTerm(QStringList& tokens) {
    // 逻辑同上，但是调用 parseFactor，处理 * / MOD
    Expression* lhs = parseExponent(tokens);
    while (!tokens.isEmpty() && (tokens.first() == "*" || tokens.first() == "/" || tokens.first() == "MOD")) {
        QString op = tokens.takeFirst();
        Expression* rhs = parseExponent(tokens);
        lhs = new CompoundExp(lhs, op.toStdString(), rhs);
    }
    return lhs;
}

Expression* Parser::parseFactor(QStringList& tokens) {
    if (tokens.isEmpty()) throw std::runtime_error("Unexpected end of expression");

    // 负数
    if (tokens.first() == "-") {
        tokens.takeFirst(); // 消耗掉 "-"
        Expression* val = parseFactor(tokens); // 递归解析后面的数字
        return new CompoundExp(new ConstantExp(0), "-", val);
    }

    QString t = tokens.takeFirst();

    if (t == "(") {
        Expression* exp = parseExpression(tokens); // 递归！括号里又是一个完整的表达式
        tokens.takeFirst(); // 消耗掉 ")"
        return exp;
    }
    // 判断是数字还是变量
    bool isNum;
    int val = t.toInt(&isNum);
    if (isNum) return new ConstantExp(val);
    else return new IdentifierExp(t.toStdString());
}

Expression* Parser::parseExponent(QStringList& tokens) {
    Expression* lhs = parseFactor(tokens);

    // 检查是否有 **
    if (!tokens.isEmpty() && tokens.first() == "**") {
        QString op = tokens.takeFirst();
        // 递归调用 parseExponent 自身，实现“右结合” (a ** (b ** c))
        Expression* rhs = parseExponent(tokens);
        lhs = new CompoundExp(lhs, op.toStdString(), rhs);
    }
    return lhs;
}
