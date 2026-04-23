#include "mainwindow.h"
#include "utils.h"

#include <QHeaderView>
#include <QFont>
#include <QColor>
#include <QBrush>
#include <QStack>
#include <QHash>
#include <QSet>
#include <QDebug>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), mainTokenCode(0)
{
    setWindowTitle(QString::fromUtf8("编译原理实验2：正则表达式转换为DFA图"));
    resize(1200, 800);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // === 顶部区域 ===
    QWidget* topWidget = new QWidget();
    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);

    QLabel* inputLabel = new QLabel(QString::fromUtf8("请输入正则表达式（每行一个定义）："));
    topLayout->addWidget(inputLabel);

    inputEdit = new QTextEdit();
    inputEdit->setMaximumHeight(150);
    inputEdit->setFont(QFont("Consolas", 11));
    inputEdit->setPlaceholderText(
        "letter=[A-Za-z]\ndigit=[0-9]\nID_101=letter(letter|digit)*");
    topLayout->addWidget(inputEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnOpen = new QPushButton(QString::fromUtf8("打开规则"));
    btnSave = new QPushButton(QString::fromUtf8("保存规则"));
    btnBuild = new QPushButton(QString::fromUtf8("构造自动机"));
    btnBuild->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 6px 20px; }");
    btnLayout->addWidget(btnOpen);
    btnLayout->addWidget(btnSave);
    btnLayout->addStretch();
    btnLayout->addWidget(btnBuild);
    topLayout->addLayout(btnLayout);

    mainLayout->addWidget(topWidget);

    // === 中部区域 ===
    tabWidget = new QTabWidget();

    nfaTable = new QTableWidget();
    nfaTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    nfaTable->setSelectionMode(QAbstractItemView::NoSelection);
    tabWidget->addTab(nfaTable, "NFA");

    dfaTable = new QTableWidget();
    dfaTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dfaTable->setSelectionMode(QAbstractItemView::NoSelection);
    tabWidget->addTab(dfaTable, "DFA");

    miniDfaTable = new QTableWidget();
    miniDfaTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    miniDfaTable->setSelectionMode(QAbstractItemView::NoSelection);
    tabWidget->addTab(miniDfaTable, QString::fromUtf8("最小化DFA"));

    lexerCodeBrowser = new QTextBrowser();
    lexerCodeBrowser->setFont(QFont("Consolas", 10));
    tabWidget->addTab(lexerCodeBrowser, QString::fromUtf8("词法分析程序"));

    mainLayout->addWidget(tabWidget, 1);

    // === 底部区域：日志 ===
    QLabel* logLabel = new QLabel(QString::fromUtf8("日志："));
    mainLayout->addWidget(logLabel);

    logBrowser = new QTextBrowser();
    logBrowser->setMaximumHeight(120);
    logBrowser->setFont(QFont("Consolas", 9));
    mainLayout->addWidget(logBrowser);

    // 信号连接
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveFile);
    connect(btnBuild, &QPushButton::clicked, this, &MainWindow::onBuildAutomata);
}

MainWindow::~MainWindow() {
}

void MainWindow::log(const QString& msg) {
    logBrowser->append(msg);
}

QString MainWindow::resolveSymbolName(const QString& symbol) const {
    if (placeholderToName.contains(symbol)) {
        return placeholderToName[symbol];
    }
    return symbol;
}

void MainWindow::onOpenFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("打开正则表达式文件"), "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            inputEdit->setPlainText(QString::fromUtf8(file.readAll()));
            file.close();
            log(QString::fromUtf8("已打开文件: ") + fileName);
        }
    }
}

void MainWindow::onSaveFile() {
    QString fileName = QFileDialog::getSaveFileName(this,
        QString::fromUtf8("保存正则表达式文件"), "", "Text Files (*.txt);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(inputEdit->toPlainText().toUtf8());
            file.close();
            log(QString::fromUtf8("已保存文件: ") + fileName);
        }
    }
}

void MainWindow::onBuildAutomata() {
    log(QString::fromUtf8("========== 开始构造自动机 =========="));

    nfa.clear();
    dfa.clear();
    miniDFA.clear();
    lexerCode.clear();
    symbolRanges.clear();
    placeholderToName.clear();
    nameToPlaceholder.clear();
    mainRegexName.clear();
    mainTokenCode = 0;

    QString allText = inputEdit->toPlainText().trimmed();
    if (allText.isEmpty()) {
        log(QString::fromUtf8("错误：输入为空！"));
        return;
    }

    QStringList lines = allText.split('\n', Qt::SkipEmptyParts);

    QString mainRegex = preprocessRegexList(lines);
    if (mainRegex.isEmpty()) {
        log(QString::fromUtf8("错误：未找到需要生成DFA的主正则表达式！"));
        return;
    }
    log(QString::fromUtf8("预处理后的主正则表达式: ") + mainRegex);

    QString postfix = regexToPostfix(mainRegex);
    log(QString::fromUtf8("后缀表达式: ") + postfix);

    nfa.fromPostfix(postfix);
    log(QString::fromUtf8("NFA构造完成，状态数: ") + QString::number(nfa.stateCount)
        + QString::fromUtf8("，起始态: ") + QString::number(nfa.startState)
        + QString::fromUtf8("，终态: ") + QString::number(nfa.endState));

    dfa.fromNFA(nfa);
    log(QString::fromUtf8("DFA构造完成，状态数: ") + QString::number(dfa.stateCount));

    miniDFA = dfa.minimize();
    log(QString::fromUtf8("最小化DFA完成，状态数: ") + QString::number(miniDFA.stateCount));

    QMap<QString, QString> codeSymbolRanges;
    for (auto it = symbolRanges.begin(); it != symbolRanges.end(); ++it) {
        codeSymbolRanges[it.key()] = it.value();
    }
    lexerCode = miniDFA.generateLexerCode(codeSymbolRanges, placeholderToName, mainRegexName, mainTokenCode);
    log(QString::fromUtf8("词法分析程序生成完成。"));

    displayNFA();
    displayDFA();
    displayMiniDFA();
    lexerCodeBrowser->setPlainText(lexerCode);

    tabWidget->setCurrentIndex(0);
    log(QString::fromUtf8("========== 构造完成 =========="));
}

QString MainWindow::preprocessRegexList(QStringList lines) {
    int lineSize = lines.size();

    for (int i = 0; i < lineSize; i++) {
        lines[i] = lines[i].trimmed();
    }

    // ===== 第一步：从原始输入中识别命名字符集定义 =====
    QSet<QString> namedCharSets;
    QString originalText = inputEdit->toPlainText();
    QStringList originalLines = originalText.split('\n', Qt::SkipEmptyParts);
    for (const QString& origLine : originalLines) {
        QString trimmed = origLine.trimmed();
        int eqPos = trimmed.indexOf('=');
        if (eqPos <= 0) continue;
        QString name = trimmed.left(eqPos).trimmed();
        QString body = trimmed.mid(eqPos + 1).trimmed();

        // 跳过主正则表达式（含下划线+数字编码）
        if (name.contains('_')) {
            int up = name.indexOf('_');
            bool hasCode = false;
            for (int k = up + 1; k < name.size(); k++) {
                if (name[k].isDigit()) { hasCode = true; break; }
            }
            if (hasCode) continue;
        }

        if (body.startsWith('[') && body.contains(']')) {
            int rb = body.indexOf(']');
            QString range = body.mid(1, rb - 1);
            symbolRanges[name] = range;
            namedCharSets.insert(name);
            log(QString::fromUtf8("记录命名字符集: ") + name + " -> [" + range + "]");
        }
    }

    // ===== 第二步：为命名字符集分配占位符 =====
    QList<QString> sortedNames = namedCharSets.values();
    std::sort(sortedNames.begin(), sortedNames.end(), [](const QString& a, const QString& b) {
        return a.size() > b.size();
        });

    ushort placeholderBase = 0xE000;
    for (const QString& name : sortedNames) {
        QString placeholder = QString(QChar(placeholderBase));
        placeholderToName[placeholder] = name;
        nameToPlaceholder[name] = placeholder;
        placeholderBase++;
        log(QString::fromUtf8("占位符映射: ") + name + " -> U+" + QString::number(placeholderBase - 1, 16));
    }

    // ===== 第三步：对非命名字符集的行展开[] =====
    for (int i = 0; i < lineSize; i++) {
        QString line = lines[i];
        int eqPos = line.indexOf('=');
        if (eqPos <= 0) continue;
        QString name = line.left(eqPos);

        if (namedCharSets.contains(name)) continue;

        int leftBracket = -1;
        for (int j = 0; j < lines[i].size(); j++) {
            if (lines[i][j] == '\\') {
                j++;
            }
            else if (lines[i][j] == '[') {
                leftBracket = j;
            }
            else if (lines[i][j] == ']') {
                if (leftBracket == -1) continue;
                if (leftBracket == j - 1) continue;

                QString bracketStr = lines[i].mid(leftBracket + 1, j - leftBracket - 1);
                QString expanded;
                int bi = 0;
                while (bi < bracketStr.size()) {
                    if (bi + 2 < bracketStr.size() && bracketStr[bi + 1] == '-') {
                        QChar from = bracketStr[bi];
                        QChar to = bracketStr[bi + 2];
                        for (char c = from.toLatin1(); c <= to.toLatin1(); c++) {
                            if (!expanded.isEmpty()) expanded += '|';
                            expanded += c;
                        }
                        bi += 3;
                    }
                    else {
                        if (!expanded.isEmpty()) expanded += '|';
                        expanded += bracketStr[bi];
                        bi++;
                    }
                }

                lines[i] = lines[i].left(leftBracket) + "(" + expanded + ")" + lines[i].mid(j + 1);
                j = leftBracket + expanded.size() + 2 - 1;
                leftBracket = -1;
            }
        }
    }

    // ===== 第四步：替换正闭包+ =====
    for (int i = 0; i < lineSize; i++) {
        int leftParen = -1;
        for (int j = 0; j < lines[i].size(); j++) {
            if (lines[i][j] == '\\') {
                j++;
            }
            else if (lines[i][j] == '(') {
                leftParen = j;
            }
            else if (lines[i][j] == '+') {
                if (j > 0 && lines[i][j - 1] != '\\') {
                    lines[i][j] = '*';
                    if (lines[i][j - 1] == ')') {
                        QString dup = lines[i].mid(leftParen, j - leftParen);
                        lines[i].insert(leftParen, dup);
                        j += dup.size();
                    }
                    else {
                        lines[i].insert(j - 1, lines[i][j - 1]);
                        j += 1;
                    }
                }
            }
        }
    }

    // ===== 第五步：替换可选? =====
    for (int i = 0; i < lineSize; i++) {
        int leftParen = -1;
        for (int j = 0; j < lines[i].size(); j++) {
            if (lines[i][j] == '\\') {
                j++;
            }
            else if (lines[i][j] == '(') {
                leftParen = j;
            }
            else if (lines[i][j] == '?') {
                if (j > 0) {
                    lines[i][j] = '|';
                    if (lines[i][j - 1] == ')') {
                        lines[i].insert(leftParen, '(');
                        lines[i].insert(j + 2, "#)");
                        j += 3;
                    }
                    else {
                        lines[i].insert(j - 1, '(');
                        lines[i].insert(j + 2, "#)");
                        j += 3;
                    }
                }
            }
        }
    }

    // ===== 第六步：分离主正则表达式和辅助定义 =====
    QString mainRegex;
    QHash<QString, QString> regexHash;

    for (int i = 0; i < lineSize; i++) {
        QString line = lines[i];
        int eqPos = line.indexOf('=');
        if (eqPos <= 0) continue;

        QString name = line.left(eqPos);
        QString body = line.mid(eqPos + 1);

        // 主正则表达式（名称中含下划线+数字）
        int underscorePos = name.indexOf('_');
        if (underscorePos >= 0 && underscorePos < name.size() - 1) {
            QString codeStr;
            for (int k = underscorePos + 1; k < name.size(); k++) {
                if (name[k].isDigit()) {
                    codeStr += name[k];
                }
                else {
                    break;
                }
            }
            if (!codeStr.isEmpty()) {
                mainTokenCode = codeStr.toInt();
                mainRegexName = name.left(underscorePos);
                mainRegex = body;
                log(QString::fromUtf8("找到主正则表达式: ") + name + " = " + body
                    + QString::fromUtf8("，编码: ") + codeStr);
                continue;
            }
        }

        // 命名字符集不加入regexHash
        if (namedCharSets.contains(name)) continue;

        // 其他辅助定义
        regexHash[name] = body;
    }

    // 备选：找以_开头的
    if (mainRegex.isEmpty()) {
        for (int i = 0; i < lineSize; i++) {
            QString line = lines[i];
            int eqPos = line.indexOf('=');
            if (eqPos <= 0) continue;
            QString name = line.left(eqPos);
            if (name.startsWith('_')) {
                mainRegex = line.mid(eqPos + 1);
                mainRegexName = name.mid(1);
                mainTokenCode = 0;
                break;
            }
        }
    }

    if (mainRegex.isEmpty()) return QString();

    log(QString::fromUtf8("第六步后 mainRegex = ") + mainRegex);

    // ===== 第七步：合并非字符集的辅助定义 =====
    for (int iter = 0; iter < 10; iter++) {
        bool replaced = false;
        for (auto it = regexHash.begin(); it != regexHash.end(); ++it) {
            QString key = it.key();
            if (mainRegex.contains(key)) {
                QString replaceStr = it.value();
                if (!replaceStr.startsWith('(') || !replaceStr.endsWith(')')) {
                    replaceStr = "(" + replaceStr + ")";
                }
                mainRegex.replace(key, replaceStr);
                replaced = true;
            }
        }
        if (!replaced) break;
    }

    log(QString::fromUtf8("第七步后 mainRegex = ") + mainRegex);

    // ===== 第八步：将命名字符集名称替换为占位符 =====
    for (const QString& name : sortedNames) {
        mainRegex.replace(name, nameToPlaceholder[name]);
    }

    log(QString::fromUtf8("第八步后 mainRegex = ") + mainRegex);

    // ===== 第九步：添加连接符 '.' =====
    QString result;
    for (int i = 0; i < mainRegex.size(); i++) {
        QChar current = mainRegex[i];
        result += current;

        if (i + 1 < mainRegex.size()) {
            QChar next = mainRegex[i + 1];

            if (current == '\\') {
                continue;
            }

            bool prevIsOperand = false;
            if (current == ')' || current == '*' || current == '#') {
                prevIsOperand = true;
            }
            else if (!isOperator(current)) {
                prevIsOperand = true;
            }

            bool nextIsOperand = false;
            if (next == '(' || next == '#') {
                nextIsOperand = true;
            }
            else if (next == '\\') {
                nextIsOperand = true;
            }
            else if (!isOperator(next)) {
                nextIsOperand = true;
            }

            if (prevIsOperand && nextIsOperand) {
                result += '.';
            }
        }
    }

    log(QString::fromUtf8("第九步后（添加连接符）: ") + result);

    return result;
}

QString MainWindow::regexToPostfix(const QString& regex) {
    QStack<QChar> opStack;
    QString result;

    for (int i = 0; i < regex.size(); i++) {
        QChar ch = regex[i];

        if (ch == '\\') {
            if (i + 1 < regex.size()) {
                result += '\\';
                result += regex[i + 1];
                i++;
            }
        }
        else if (ch == '(') {
            opStack.push(ch);
        }
        else if (ch == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                result += opStack.top();
                opStack.pop();
            }
            if (!opStack.empty() && opStack.top() == '(') {
                opStack.pop();
            }
        }
        else if (isOperator(ch)) {
            int currentPriority = getPriority(ch);
            while (!opStack.empty() && opStack.top() != '(' &&
                getPriority(opStack.top()) >= currentPriority) {
                result += opStack.top();
                opStack.pop();
            }
            opStack.push(ch);
        }
        else {
            result += ch;
        }
    }

    while (!opStack.empty()) {
        result += opStack.top();
        opStack.pop();
    }

    return result;
}

void MainWindow::displayNFA() {
    QList<QString> columns = nfa.orderedAlphabet;
    columns.append("epsilon");

    int colCount = 2 + columns.size();
    int rowCount = nfa.stateCount;

    nfaTable->setRowCount(rowCount);
    nfaTable->setColumnCount(colCount);

    QStringList headers;
    headers << "" << "State";
    for (const QString& sym : columns) {
        if (sym == "epsilon")
            headers << QString::fromUtf8("ε");
        else
            headers << resolveSymbolName(sym);
    }
    nfaTable->setHorizontalHeaderLabels(headers);

    nfaTable->setColumnWidth(0, 40);
    nfaTable->setColumnWidth(1, 60);
    for (int c = 2; c < colCount; c++) {
        nfaTable->setColumnWidth(c, 100);
    }
    nfaTable->verticalHeader()->setVisible(false);

    for (int s = 0; s < nfa.stateCount; s++) {
        QTableWidgetItem* markItem = new QTableWidgetItem();
        if (s == nfa.startState && s == nfa.endState) {
            markItem->setBackground(QBrush(QColor(255, 255, 0)));
        }
        else if (s == nfa.startState) {
            markItem->setBackground(QBrush(QColor(0, 200, 0)));
        }
        else if (s == nfa.endState) {
            markItem->setBackground(QBrush(QColor(255, 80, 80)));
        }
        nfaTable->setItem(s, 0, markItem);

        QTableWidgetItem* stateItem = new QTableWidgetItem(QString::number(s + 1));
        stateItem->setTextAlignment(Qt::AlignCenter);
        nfaTable->setItem(s, 1, stateItem);

        for (int c = 0; c < columns.size(); c++) {
            QString symbol = columns[c];
            QStringList targets;
            for (const auto& edge : nfa.edges) {
                if (edge.from == s && edge.symbol == symbol) {
                    targets.append(QString::number(edge.to + 1));
                }
            }
            QTableWidgetItem* item = new QTableWidgetItem(targets.join(", "));
            item->setTextAlignment(Qt::AlignCenter);
            nfaTable->setItem(s, c + 2, item);
        }
    }
}

void MainWindow::displayDFA() {
    QList<QString> columns = dfa.alphabet;

    int colCount = 2 + columns.size();
    int rowCount = dfa.stateCount;

    dfaTable->setRowCount(rowCount);
    dfaTable->setColumnCount(colCount);

    QStringList headers;
    headers << "" << QString::fromUtf8("状态集合");
    for (const QString& sym : columns) {
        headers << resolveSymbolName(sym);
    }
    dfaTable->setHorizontalHeaderLabels(headers);

    dfaTable->setColumnWidth(0, 40);
    dfaTable->setColumnWidth(1, 150);
    for (int c = 2; c < colCount; c++) {
        dfaTable->setColumnWidth(c, 150);
    }
    dfaTable->verticalHeader()->setVisible(false);

    for (int s = 0; s < dfa.stateCount; s++) {
        QTableWidgetItem* markItem = new QTableWidgetItem();
        bool isStart = (s == dfa.startState);
        bool isEnd = dfa.endStates.contains(s);
        if (isStart && isEnd) {
            markItem->setBackground(QBrush(QColor(255, 255, 0)));
        }
        else if (isStart) {
            markItem->setBackground(QBrush(QColor(0, 200, 0)));
        }
        else if (isEnd) {
            markItem->setBackground(QBrush(QColor(255, 80, 80)));
        }
        dfaTable->setItem(s, 0, markItem);

        QSet<int> nfaStates = dfa.mapping[s];
        QList<int> stateList = nfaStates.values();
        std::sort(stateList.begin(), stateList.end());
        QStringList stateStrs;
        for (int ns : stateList) {
            stateStrs.append(QString::number(ns + 1));
        }
        QString setStr = "{" + stateStrs.join(", ") + "}";
        QTableWidgetItem* stateItem = new QTableWidgetItem(setStr);
        stateItem->setTextAlignment(Qt::AlignCenter);
        dfaTable->setItem(s, 1, stateItem);

        for (int c = 0; c < columns.size(); c++) {
            QString symbol = columns[c];
            QString cellText;
            if (dfa.G.contains(s) && dfa.G[s].contains(symbol)) {
                int target = dfa.G[s][symbol];
                QSet<int> targetNFA = dfa.mapping[target];
                QList<int> targetList = targetNFA.values();
                std::sort(targetList.begin(), targetList.end());
                QStringList targetStrs;
                for (int ns : targetList) {
                    targetStrs.append(QString::number(ns + 1));
                }
                cellText = "{" + targetStrs.join(", ") + "}";
            }
            QTableWidgetItem* item = new QTableWidgetItem(cellText);
            item->setTextAlignment(Qt::AlignCenter);
            dfaTable->setItem(s, c + 2, item);
        }
    }
}

void MainWindow::displayMiniDFA() {
    QList<QString> columns = miniDFA.alphabet;

    int colCount = 2 + columns.size();
    int rowCount = miniDFA.stateCount;

    miniDfaTable->setRowCount(rowCount);
    miniDfaTable->setColumnCount(colCount);

    QStringList headers;
    headers << "" << "State";
    for (const QString& sym : columns) {
        headers << resolveSymbolName(sym);
    }
    miniDfaTable->setHorizontalHeaderLabels(headers);

    miniDfaTable->setColumnWidth(0, 40);
    miniDfaTable->setColumnWidth(1, 60);
    for (int c = 2; c < colCount; c++) {
        miniDfaTable->setColumnWidth(c, 100);
    }
    miniDfaTable->verticalHeader()->setVisible(false);

    for (int s = 0; s < miniDFA.stateCount; s++) {
        QTableWidgetItem* markItem = new QTableWidgetItem();
        bool isStart = (s == miniDFA.startState);
        bool isEnd = miniDFA.endStates.contains(s);
        if (isStart && isEnd) {
            markItem->setBackground(QBrush(QColor(255, 255, 0)));
        }
        else if (isStart) {
            markItem->setBackground(QBrush(QColor(0, 200, 0)));
        }
        else if (isEnd) {
            markItem->setBackground(QBrush(QColor(255, 80, 80)));
        }
        miniDfaTable->setItem(s, 0, markItem);

        QTableWidgetItem* stateItem = new QTableWidgetItem(QString::number(s + 1));
        stateItem->setTextAlignment(Qt::AlignCenter);
        miniDfaTable->setItem(s, 1, stateItem);

        for (int c = 0; c < columns.size(); c++) {
            QString symbol = columns[c];
            QString cellText;
            if (miniDFA.G.contains(s) && miniDFA.G[s].contains(symbol)) {
                cellText = QString::number(miniDFA.G[s][symbol] + 1);
            }
            QTableWidgetItem* item = new QTableWidgetItem(cellText);
            item->setTextAlignment(Qt::AlignCenter);
            miniDfaTable->setItem(s, c + 2, item);
        }
    }
}