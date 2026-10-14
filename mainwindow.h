#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QEventLoop>
#include "expression.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int getUserInput();

private slots:
    void handleInput();
    void runProgram();
    void handleLoad();
    void handleClear();
    void handleRun();

private:
    Ui::MainWindow *ui;
    QMap<int, QString> programCode;
    EvaluationContext context;
    bool isWaitingForInput = false;
    int inputVal = 0;
    QEventLoop *inputLoop;

    void clearAll();
    void refreshCodeDisplay();
};
#endif // MAINWINDOW_H
