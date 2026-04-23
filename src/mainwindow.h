#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

#include "nfa.h"
#include "dfa.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFile();
    void onSaveFile();
    void onBuildAutomata();

private:
    // UI组件
    QTextEdit* inputEdit;
    QPushButton* btnOpen;
    QPushButton* btnSave;
    QPushButton* btnBuild;
    QTabWidget* tabWidget;
    QTableWidget* nfaTable;
    QTableWidget* dfaTable;
    QTableWidget* miniDfaTable;
    QTextBrowser* lexerCodeBrowser;
    QTextBrowser* logBrowser;

    // 数据
    NFA nfa;
    DFA dfa;
    DFA miniDFA;
    QString lexerCode;

    // 记录符号名称和范围的映射
    QMap<QString, QString> symbolRanges;
    QString mainRegexName;
    int mainTokenCode;

    // 占位符映射：单个Unicode字符 <-> 命名字符集名称
    QHash<QString, QString> placeholderToName;  // 占位符字符串 -> 名称（如 "letter"）
    QHash<QString, QString> nameToPlaceholder;  // 名称 -> 占位符字符串

    // 方法
    QString preprocessRegexList(QStringList lines);
    QString regexToPostfix(const QString& regex);
    void displayNFA();
    void displayDFA();
    void displayMiniDFA();

    // 将符号中的占位符还原为名称（用于显示）
    QString resolveSymbolName(const QString& symbol) const;

    void log(const QString& msg);
};

#endif // MAINWINDOW_H