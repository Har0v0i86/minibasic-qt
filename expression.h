#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <map>
#include <stdexcept>
#include <functional>


struct EvaluationContext {
    std::map<std::string, int> vars;
    std::map<std::string, int> varUseCounts;

    // 这是一个函数指针，用于把 PRINT 的内容传给 GUI
    std::function<void(std::string)> outputCallback;
    std::function<int()> inputCallback;

    void setValue(std::string var, int value) {
        vars[var] = value;
    }

    int getValue(std::string var) {
        if (vars.find(var) == vars.end()) {
            throw std::runtime_error("Identifier not difined " + var);
        }
        return vars[var];
    }

    void addVarUsage(std::string var) {
        varUseCounts[var]++;
    }

    int getVarUseCount(std::string var) {
        return varUseCounts[var];
    }

    // 供 PRINT 语句调用
    void print(std::string msg) {
        if (outputCallback) {
            outputCallback(msg);
        }
    }

    int input() {
        if (inputCallback) {
            return inputCallback();
        }
        throw std::runtime_error("Input callback not set!");
    }

    void clear() {
        vars.clear();
        varUseCounts.clear();
    }
};



enum ExpressionType { CONSTANT, IDENTIFIER, COMPOUND };

class Expression {
public:
    virtual ~Expression() {}
    virtual int eval(EvaluationContext &context) = 0;
    virtual ExpressionType type() = 0;
    virtual std::string toString() = 0;
    virtual std::string getTreeString(int level) = 0;
};

class ConstantExp : public Expression {
public:
    ConstantExp(int val);
    virtual int eval(EvaluationContext &context) override;
    virtual ExpressionType type() override;
    virtual std::string toString() override;
    virtual std::string getTreeString(int level) override;
private:
    int value;
};

class IdentifierExp : public Expression {
public:
    IdentifierExp(std::string name);
    virtual int eval(EvaluationContext &context) override;
    virtual ExpressionType type() override;
    virtual std::string toString() override;
    virtual std::string getTreeString(int level) override;
private:
    std::string name;
};

class CompoundExp : public Expression {
public:
    CompoundExp(Expression *lhs, std::string op, Expression *rhs);
    virtual ~CompoundExp();
    virtual int eval(EvaluationContext &context) override;
    virtual ExpressionType type() override;
    virtual std::string toString() override;
    virtual std::string getTreeString(int level) override;
private:
    Expression *lhs;
    std::string op;
    Expression *rhs;
};

#endif // EXPRESSION_H
