#include "expression.h"

std::string getIndent(int level) {
    return std::string(level * 4, ' ');
}

// 常数表达式
ConstantExp::ConstantExp(int val) : value(val) {}

int ConstantExp::eval(EvaluationContext &context)
{
    return value;
}

std::string ConstantExp::getTreeString(int level) {
    // 显示: 缩进 + 数值
    return getIndent(level) + std::to_string(value) + "\n";
}

ExpressionType ConstantExp::type() { return CONSTANT; }

std::string ConstantExp::toString() { return std::to_string(value); }

// 变量表达式
IdentifierExp::IdentifierExp(std::string name) : name(name) {}

int IdentifierExp::eval(EvaluationContext &context)
{
    context.addVarUsage(name);
    return context.getValue(name);
}

std::string IdentifierExp::getTreeString(int level) {
    // 显示: 缩进 + 变量名
    return getIndent(level) + name + "\n";
}

ExpressionType IdentifierExp::type() { return IDENTIFIER; }

std::string IdentifierExp::toString() { return name; }

// 复合表达式
CompoundExp::CompoundExp(Expression *lhs, std::string op, Expression *rhs)
    : lhs(lhs), op(op), rhs(rhs) {}

CompoundExp::~CompoundExp()
{
    delete lhs;
    delete rhs;
}

int CompoundExp::eval(EvaluationContext &context)
{
    int leftVal = lhs->eval(context);
    int rightVal = rhs->eval(context);

    if (op == "+") return leftVal + rightVal;
    if (op == "-") return leftVal - rightVal;
    if (op == "*") return leftVal * rightVal;
    if (op == "/") {
        if (rightVal == 0) throw std::runtime_error("Division by zero");
        return leftVal / rightVal;
    }
    if (op == "**") {
        int res = 1;
        for(int i=0; i<rightVal; ++i) res *= leftVal;
        return res;
    }
    if (op == "MOD") {
        if (rightVal == 0) throw std::runtime_error("Modulo by zero");
        int res = leftVal % rightVal;
        // BASIC 要求结果符号随除数(divisor)，且绝对值小于除数
        if ((res > 0 && rightVal < 0) || (res < 0 && rightVal > 0)) {
            res += rightVal;
        }
        return res;
    }
    throw std::runtime_error("Unknown operator: " + op);
}

std::string CompoundExp::getTreeString(int level) {
    // 缩进 + 运算符
    // 缩进(下一级) + 左子树
    // 缩进(下一级) + 右子树
    std::string str = getIndent(level) + op + "\n";
    str += lhs->getTreeString(level + 1);
    str += rhs->getTreeString(level + 1);
    return str;
}

ExpressionType CompoundExp::type() { return COMPOUND; }

std::string CompoundExp::toString()
{
    return "(" + lhs->toString() + " " + op + " " + rhs->toString() + ")";
}
