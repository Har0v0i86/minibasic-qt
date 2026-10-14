#ifndef STATEMENT_H
#define STATEMENT_H

#include "expression.h"
#include <string>

// === 语句基类 (Statement) ===
class Statement {
public:
    virtual ~Statement() {}

    // 返回值说明：
    //   -1 : 顺序执行下一行
    //   >0 : 跳转到该行号 (GOTO/IF)
    //   -2 : 程序结束 (END)
    virtual int execute(EvaluationContext &context) = 0;
    //  语法树生成，需要传入 Context 以获取变量使用统计
    virtual std::string getTreeString(EvaluationContext &context, int level) = 0;
    //  重置计数器
    virtual void reset() { execCount = 0; }
protected:
    int execCount = 0; // 语句执行计数
};

//  REM (注释)
class RemStmt : public Statement {
public:
    RemStmt(std::string c);
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
private:
    std::string comment;
};

//  LET (赋值)
// 语法: LET variable = expression
class LetStmt : public Statement {
public:
    LetStmt(std::string varName, Expression *exp);
    virtual ~LetStmt(); // 需要负责释放 expression 内存
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
private:
    std::string variable;
    Expression *expression;
};

//  PRINT (打印)
// 语法: PRINT expression
class PrintStmt : public Statement {
public:
    PrintStmt(Expression *exp);
    virtual ~PrintStmt();
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
private:
    Expression *expression;
};

//  INPUT (输入)
// 语法: INPUT variable
class InputStmt : public Statement {
public:
    InputStmt(std::string varName);
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
private:
    std::string variableName;
};

//  GOTO (跳转)
// 语法: GOTO lineNumber
class GotoStmt : public Statement {
public:
    GotoStmt(int line);
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
private:
    int lineNumber;
};

//  IF (条件跳转)
// 语法: IF expression op expression THEN lineNumber
class IfStmt : public Statement {
public:
    IfStmt(Expression *l, std::string op, Expression *r, int line);
    virtual ~IfStmt();
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
    virtual void reset() override; // 需要重置两个计数器
private:
    Expression *lhs;
    std::string op;
    Expression *rhs;
    int targetLine;
    int trueCount = 0;  // 跳转次数
    int falseCount = 0; // 不跳转次数
};

//  END (结束)
// 语法: END
class EndStmt : public Statement {
public:
    EndStmt();
    virtual int execute(EvaluationContext &context) override;
    virtual std::string getTreeString(EvaluationContext &context, int level) override;
};

#endif // STATEMENT_H
