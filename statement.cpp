#include "statement.h"

std::string getIndentS(int level) {
    return std::string(level * 4, ' ');
}

// Rem语句

RemStmt::RemStmt(std::string c) : comment(c) {}

int RemStmt::execute(EvaluationContext &context)
{
    execCount++;
    return -1;
}

std::string RemStmt::getTreeString(EvaluationContext &context, int level)
{
    // REM 1 (若执行一次)
    return getIndentS(level) + "REM " + std::to_string(execCount) + "\n" +
           getIndentS(level) + comment + "\n";
}

// Let语句
LetStmt::LetStmt(std::string varName, Expression *exp)
    : variable(varName), expression(exp) {}

LetStmt::~LetStmt()
{
    delete expression;
}

int LetStmt::execute(EvaluationContext &context)
{
    execCount++;
    // 计算表达式的值
    int val = expression->eval(context);
    // 将值存入上下文（变量表）
    context.setValue(variable, val);
    return -1;
}

std::string LetStmt::getTreeString(EvaluationContext &context, int level) {

    std::string str = getIndentS(level) + "LET = " + std::to_string(execCount) + "\n";

    // 获取左值变量的使用次数
    int varCount = context.getVarUseCount(variable);
    str += getIndentS(level + 1) + variable + " " + std::to_string(varCount) + "\n";

    // 递归打印右侧表达式
    str += expression->getTreeString(level + 1);
    return str;
}

// Print语句
PrintStmt::PrintStmt(Expression *exp) : expression(exp) {}

PrintStmt::~PrintStmt()
{
    delete expression;
}

int PrintStmt::execute(EvaluationContext &context)
{
    execCount++;
    // 计算表达式的值
    int val = expression->eval(context);
    // 将结果输出到 GUI 界面
    context.print(std::to_string(val) + "\n");
    return -1;
}

std::string PrintStmt::getTreeString(EvaluationContext &context, int level)
{
    std::string str = getIndentS(level) + "PRINT " + std::to_string(execCount) + "\n";
    str += expression->getTreeString(level + 1);
    return str;
}

// Input语句
InputStmt::InputStmt(std::string varName) : variableName(varName) {}

int InputStmt::execute(EvaluationContext &context)
{
    execCount++;
    // 1. 打印提示符 " ? "
    context.print(" ? ");

    // 2. 调用回调函数获取输入
    int val = context.input();

    // 3. 将获取到的值存入变量
    context.setValue(variableName, val);

    return -1;
}

std::string InputStmt::getTreeString(EvaluationContext &context, int level)
{
    std::string str = getIndentS(level) + "INPUT " + std::to_string(execCount) + "\n";
    str += getIndentS(level + 1) + variableName + "\n";
    return str;
}

// Goto语句
GotoStmt::GotoStmt(int line) : lineNumber(line) {}

int GotoStmt::execute(EvaluationContext &context)
{
    execCount++;
    return lineNumber;
}

std::string GotoStmt::getTreeString(EvaluationContext &context, int level)
{
    std::string str = getIndentS(level) + "GOTO " + std::to_string(execCount) + "\n";
    str += getIndentS(level + 1) + std::to_string(lineNumber) + "\n";
    return str;
}

// If语句
IfStmt::IfStmt(Expression *l, std::string op, Expression *r, int line)
    : lhs(l), op(op), rhs(r), targetLine(line) {}

IfStmt::~IfStmt()
{
    delete lhs;
    delete rhs;
}

int IfStmt::execute(EvaluationContext &context)
{
    execCount++;
    int left = lhs->eval(context);
    int right = rhs->eval(context);
    bool condition = false;

    // 根据操作符判断条件是否成立
    if (op == "=") condition = (left == right);
    else if (op == "<") condition = (left < right);
    else if (op == ">") condition = (left > right);
    // 如果需要支持 <=, >=, <> 等，可以在这里添加

    if (condition) {
        trueCount++;
        return targetLine; // 条件成立，跳转到目标行
    } else {
        falseCount++;
        return -1; // 条件不成立，忽略，继续下一行
    }
}

std::string IfStmt::getTreeString(EvaluationContext &context, int level) {
    // 格式: IF THEN [trueCount] [falseCount]
    std::string header = "IF THEN " + std::to_string(trueCount) + " " + std::to_string(falseCount);
    std::string str = getIndentS(level) + header + "\n";

    str += lhs->getTreeString(level + 1);
    str += getIndentS(level + 1) + op + "\n";
    str += rhs->getTreeString(level + 1);
    str += getIndentS(level + 1) + std::to_string(targetLine) + "\n";
    return str;
}

void IfStmt::reset() {
    execCount = 0;
    trueCount = 0;
    falseCount = 0;
}

// End语句
EndStmt::EndStmt() {}

int EndStmt::execute(EvaluationContext &context)
{
    execCount++;
    return -2; // 返回 -2 表示程序应当立即停止
}

std::string EndStmt::getTreeString(EvaluationContext &context, int level) {
    return getIndentS(level) + "END " + std::to_string(execCount) + "\n";
}
