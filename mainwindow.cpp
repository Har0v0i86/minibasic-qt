#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statement.h"
#include "parser.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnClearCode,SIGNAL(clicked()),this,SLOT(handleClear()));
    connect(ui->cmdLineEdit,SIGNAL(returnPressed()),this,SLOT(handleInput()));
    connect(ui->btnLoadCode,SIGNAL(clicked()),this,SLOT(handleLoad()));
    connect(ui->btnRunCode,SIGNAL(clicked()),this,SLOT(handleRun()));

    context.outputCallback = [this](std::string msg) {
        this->ui->textBrowser->moveCursor(QTextCursor::End);
        this->ui->textBrowser->insertPlainText(QString::fromStdString(msg));
    };

    context.inputCallback = [this]() -> int {
        return this->getUserInput();
    };
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleInput()
{
    QString input = ui->cmdLineEdit->text().trimmed()   ;
    if(input.isEmpty()) return;
    ui->cmdLineEdit->clear();

    if (isWaitingForInput) {
        // --- 如果正在等待 INPUT 输入 ---
        bool ok;
        int val = input.toInt(&ok);
        if(ok) {
            inputVal = val;       // 保存输入值
            isWaitingForInput = false;

            ui->textBrowser->moveCursor(QTextCursor::End);
            ui->textBrowser->insertPlainText(input + "\n");

            ui->cmdLineEdit->clear();

            // 退出事件循环，
            if(inputLoop && inputLoop->isRunning()) {
                inputLoop->quit();
            }
        }
        return;
    }

    QStringList parts = input.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    bool isNumber;
    int lineNumber = parts[0].toInt(&isNumber);
    if (isNumber) {

        if (parts.size() == 1) {
            // 如果只输入了行号 ，删除该行
            if(programCode.contains(lineNumber)){
                programCode.remove(lineNumber);
            }
        } else {
            // 提取行号后面的实际代码内容
            int firstSpaceIndex = input.indexOf(QRegularExpression("\\s"));
            if (firstSpaceIndex != -1) {
                QString codeContent = input.mid(firstSpaceIndex).trimmed();
                // 存入 map
                programCode[lineNumber] = codeContent;
            }
        }
        refreshCodeDisplay();

    } else {
        // 输入的是直接命令或不带行号的语句
        QString command = input.toUpper();

        if (command == "RUN") {
            runProgram();
        }
        else if (command == "LOAD") {
            handleLoad();
        }
        else if (command == "CLEAR") {
            handleClear();
        }
        else if (command == "QUIT" || command == "EXIT") {
            close();
        }
        else if (command == "HELP") {
            ui->textBrowser->append("Available Commands: RUN, LOAD, CLEAR, QUIT");
        }
        else {
            //即时执行模式
            Parser parser;
            try {
                Statement* stmt = parser.parseStatement(input);
                if (stmt) {
                    // 立即执行
                    stmt->execute(context);
                    // 执行完立即销毁，不存入 programCode
                    delete stmt;
                } else {
                    // 如果解析返回 nullptr (例如空行)，则忽略
                }
            } catch (std::exception &e) {
                // 解析失败或执行失败，才认为是无效命令
                ui->textBrowser->append("Error: " + QString(e.what()));
            }
        }
    }
}


void MainWindow::runProgram()
{
    // 1. 清空旧的显示和上下文状态
    ui->textBrowser->clear();
    ui->treeDisplay->clear();
    context.clear(); // 清空变量和变量使用计数

    if (programCode.isEmpty()) return;

    Parser parser;

    // 预解析
    // 我们需要一个缓存来确保持有 Statement 对象，这样才能统计 execCount
    QMap<int, Statement*> codeMap;
    bool parseError = false;

    // 遍历 programCode (QMap 自动按行号排序)
    for (auto it = programCode.begin(); it != programCode.end(); ++it) {
        int lineNum = it.key();
        QString source = it.value();
        try {
            Statement* stmt = parser.parseStatement(source);
            if (stmt) {
                codeMap.insert(lineNum, stmt);
            }
        } catch (std::exception &e) {
            ui->textBrowser->append("Syntax Error at line " + QString::number(lineNum) + ": " + e.what());
            parseError = true;
            break;
        }
    }

    // 如果解析出错，清理已分配的内存并退出
    if (parseError) {
        qDeleteAll(codeMap);
        return;
    }
    // 执行
    int currentLine = codeMap.firstKey();

    // 只要当前行存在于 map 中，就继续执行
    while (codeMap.contains(currentLine)) {
        Statement *stmt = codeMap[currentLine];
        try {
            // 执行语句，返回下一行的行号或特殊状态码
            // 注意：stmt->execute 内部会自动增加 execCount
            int result = stmt->execute(context);

            if (result == -2) {
                break; // END 语句
            } else if (result != -1) {
                // 跳转逻辑 (GOTO / IF)
                if (!codeMap.contains(result)) {
                    throw std::runtime_error("Line number not found: " + std::to_string(result));
                }
                currentLine = result;
            } else {
                // 顺序执行：找到 map 中的下一个 key
                auto it = codeMap.find(currentLine);
                ++it; // 移到下一个
                if (it == codeMap.end()) break; // 代码结束
                currentLine = it.key();
            }

        } catch (std::exception &e) {
            ui->textBrowser->append("Runtime Error at line " + QString::number(currentLine) + ": " + e.what());
            break;
        }
    }

    // 显示语法树与统计信息
    for (auto it = codeMap.begin(); it != codeMap.end(); ++it) {
        int lineNum = it.key();
        Statement* stmt = it.value();

        // 生成这一行的树状字符串，传入 context 用于获取变量计数
        // 注意：这里的 getTreeString 是刚才让你实现的新函数
        QString treeStr = QString::fromStdString(stmt->getTreeString(context, 0));

        while (treeStr.endsWith('\n')) {
            treeStr.chop(1); // 删除最后一个字符
        }
        // 拼接行号并在 UI 显示
        // 例如: "10 REM 1\n    Program..."
        ui->treeDisplay->append(QString::number(lineNum) + " " + treeStr);
    }


    // 清理内存
    qDeleteAll(codeMap); // 删除所有 new 出来的 Statement 对象
    codeMap.clear();
}

void MainWindow::handleLoad()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text Files (*.bas)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    clearAll(); // 加载新文件前先清空

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 解析文件中的每一行
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            bool isNumber;
            int lineNumber = parts[0].toInt(&isNumber);

            if (isNumber && parts.size() > 1) {
                // 提取代码内容
                int firstSpaceIndex = line.indexOf(QRegularExpression("\\s"));
                if (firstSpaceIndex != -1) {
                    QString codeContent = line.mid(firstSpaceIndex).trimmed();
                    programCode[lineNumber] = codeContent;
                }
            }
        }
    }

    file.close();
    refreshCodeDisplay();
}


void MainWindow::handleClear()
{
    clearAll();
}

void MainWindow::refreshCodeDisplay()
{
    ui->CodeDisplay->clear();

    // 遍历 QMap，它保证了是按行号从小到大顺序遍历的
    QMap<int, QString>::const_iterator i;
    for (i = programCode.constBegin(); i != programCode.constEnd(); ++i) {
        QString lineStr = QString::number(i.key()) + " " + i.value();
        ui->CodeDisplay->append(lineStr);
    }
}

void MainWindow::clearAll()
{
    programCode.clear();

    context.clear();

    ui->CodeDisplay->clear();
    ui->textBrowser->clear();
    ui->treeDisplay->clear();
}

void MainWindow::handleRun() {
    runProgram();
}

int MainWindow::getUserInput() {
    ui->cmdLineEdit->setFocus();    // 让光标自动跳到输入框

    isWaitingForInput = true;

    QEventLoop loop;      // 创建局部循环
    inputLoop = &loop;    // 记录指针供槽函数使用
    loop.exec();          // 程序执行会在这里“暂停”，直到 loop.quit() 被调用

    inputLoop = nullptr;
    return inputVal;
}


