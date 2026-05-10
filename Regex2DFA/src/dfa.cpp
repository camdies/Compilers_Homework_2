#include "dfa.h"
#include <QQueue>
#include <QDebug>
#include <algorithm>

DFA::DFA() : startState(0), stateCount(0) {
}

void DFA::clear() {
    G.clear();
    mapping.clear();
    endStates.clear();
    stateCount = 0;
    startState = 0;
    alphabet.clear();
}

void DFA::fromNFA(const NFA& nfa) {
    clear();
    alphabet = nfa.orderedAlphabet;

    QHash<QSet<int>, int> revMapping;

    QSet<int> startSet;
    startSet.insert(nfa.startState);
    startSet = nfa.epsilonClosure(startSet);

    mapping[0] = startSet;
    revMapping[startSet] = 0;
    startState = 0;
    stateCount = 1;

    if (startSet.contains(nfa.endState)) {
        endStates.insert(0);
    }

    QQueue<int> worklist;
    worklist.enqueue(0);
    QSet<QSet<int>> visited;

    while (!worklist.empty()) {
        int current = worklist.dequeue();
        QSet<int> currentNFAStates = mapping[current];

        if (visited.contains(currentNFAStates)) continue;
        visited.insert(currentNFAStates);

        for (const QString& symbol : alphabet) {
            QSet<int> moveResult = nfa.move(currentNFAStates, symbol);
            QSet<int> closure = nfa.epsilonClosure(moveResult);

            if (closure.isEmpty()) continue;

            if (revMapping.contains(closure)) {
                int existingState = revMapping[closure];
                G[current][symbol] = existingState;
                if (!visited.contains(closure)) {
                    worklist.enqueue(existingState);
                }
            }
            else {
                int newState = stateCount++;
                mapping[newState] = closure;
                revMapping[closure] = newState;
                G[current][symbol] = newState;

                if (closure.contains(nfa.endState)) {
                    endStates.insert(newState);
                }
                if (!visited.contains(closure)) {
                    worklist.enqueue(newState);
                }
            }
        }
    }
}

DFA DFA::minimize() const {
    DFA result;
    result.alphabet = this->alphabet;

    if (stateCount == 0) return result;

    QSet<int> nonEndStates;
    for (int i = 0; i < stateCount; i++) {
        if (!endStates.contains(i)) {
            nonEndStates.insert(i);
        }
    }

    QList<QSet<int>> partition;
    if (!nonEndStates.isEmpty()) partition.append(nonEndStates);
    if (!endStates.isEmpty()) partition.append(endStates);

    bool changed = true;
    while (changed) {
        changed = false;
        QList<QSet<int>> newPartition;

        for (const QSet<int>& group : partition) {
            if (group.size() <= 1) {
                newPartition.append(group);
                continue;
            }

            bool split = false;
            for (const QString& symbol : alphabet) {
                QMap<int, QSet<int>> groupMap;
                for (int state : group) {
                    int target = -1;
                    if (G.contains(state) && G[state].contains(symbol)) {
                        target = G[state][symbol];
                    }
                    int groupIdx = -1;
                    if (target != -1) {
                        for (int gi = 0; gi < partition.size(); gi++) {
                            if (partition[gi].contains(target)) {
                                groupIdx = gi;
                                break;
                            }
                        }
                    }
                    groupMap[groupIdx].insert(state);
                }

                if (groupMap.size() > 1) {
                    for (auto it = groupMap.begin(); it != groupMap.end(); ++it) {
                        newPartition.append(it.value());
                    }
                    split = true;
                    changed = true;
                    break;
                }
            }

            if (!split) {
                newPartition.append(group);
            }
        }

        partition = newPartition;
    }

    QHash<QSet<int>, int> groupToState;
    int idx = 0;
    for (const QSet<int>& group : partition) {
        if (group.isEmpty()) continue;
        groupToState[group] = idx;
        result.mapping[idx] = group;
        idx++;
    }
    result.stateCount = idx;

    for (const QSet<int>& group : partition) {
        if (group.contains(startState)) {
            result.startState = groupToState[group];
            break;
        }
    }

    for (const QSet<int>& group : partition) {
        for (int es : endStates) {
            if (group.contains(es)) {
                result.endStates.insert(groupToState[group]);
                break;
            }
        }
    }

    for (const QSet<int>& group : partition) {
        if (group.isEmpty()) continue;
        int representative = *group.begin();
        int fromState = groupToState[group];

        for (const QString& symbol : alphabet) {
            if (G.contains(representative) && G[representative].contains(symbol)) {
                int target = G[representative][symbol];
                for (const QSet<int>& targetGroup : partition) {
                    if (targetGroup.contains(target)) {
                        result.G[fromState][symbol] = groupToState[targetGroup];
                        break;
                    }
                }
            }
        }
    }

    return result;
}

QString DFA::generateLexerCode(const QMap<QString, QString>& symbolRanges,
    const QHash<QString, QString>& placeholderToName,
    const QString& mainRegexName,
    int tokenCode) const {
    QString code;
    code += "#include <iostream>\n";
    code += "using namespace std;\n\n";

    // 为每个字符范围定义生成Judge函数
    for (auto it = symbolRanges.begin(); it != symbolRanges.end(); ++it) {
        QString symName = it.key();
        QString rangeStr = it.value();

        code += "int Judge" + symName + "(char ch)\n";
        code += "{\n";

        QString condition;
        int ci = 0;
        bool first = true;
        while (ci < rangeStr.size()) {
            if (ci + 2 < rangeStr.size() && rangeStr[ci + 1] == '-') {
                if (!first) condition += " || ";
                condition += "ch>='" + QString(rangeStr[ci]) + "' && ch<='" + QString(rangeStr[ci + 2]) + "'";
                ci += 3;
                first = false;
            }
            else {
                if (!first) condition += " || ";
                condition += "ch=='" + QString(rangeStr[ci]) + "'";
                ci++;
                first = false;
            }
        }

        code += "  if ( " + condition + " )\n";
        code += "      return 1;\n";
        code += "  else\n";
        code += "      return 0;\n";
        code += "}\n\n";
    }

    code += "int pos;\n";
    code += "char ch;\n";
    code += "char buffer[255];\n\n";

    code += "char GetNext()\n";
    code += "{   return buffer[pos++];  }\n\n";

    code += "void GetToken()\n";
    code += "{\n";
    code += "    int state = " + QString::number(startState + 1) + ";\n";
    code += "    while (1)\n";
    code += "    {\n";
    code += "        switch(state)\n";
    code += "        {\n";

    for (int s = 0; s < stateCount; s++) {
        code += "        case " + QString::number(s + 1) + ":\n";

        if (!G.contains(s) || G[s].isEmpty()) {
            if (endStates.contains(s)) {
                code += "            { cout<<\"" + QString::number(tokenCode) + "\"; return; }\n";
            }
            else {
                code += "            { cout<<\"Error!\"; return; }\n";
            }
        }
        else {
            QMap<int, QStringList> targetToSymbols;
            for (auto it = G[s].begin(); it != G[s].end(); ++it) {
                targetToSymbols[it.value()].append(it.key());
            }

            bool firstIf = true;
            for (auto it = targetToSymbols.begin(); it != targetToSymbols.end(); ++it) {
                int target = it.key();
                QStringList syms = it.value();

                QString cond;
                for (int si = 0; si < syms.size(); si++) {
                    if (si > 0) cond += "||";
                    // 检查该符号是否是占位符（对应命名字符集）
                    QString resolvedName;
                    if (placeholderToName.contains(syms[si])) {
                        resolvedName = placeholderToName[syms[si]];
                    }
                    if (!resolvedName.isEmpty() && symbolRanges.contains(resolvedName)) {
                        cond += "(Judge" + resolvedName + "(ch)==1)";
                    }
                    else {
                        cond += "(ch=='" + syms[si] + "')";
                    }
                }

                if (firstIf) {
                    code += "            if (" + cond + ")\n";
                    firstIf = false;
                }
                else {
                    code += "            else if (" + cond + ")\n";
                }
                code += "                { state=" + QString::number(target + 1) + "; ch=GetNext(); }\n";
            }

            if (endStates.contains(s)) {
                code += "            else\n";
                code += "                { cout<<\"" + QString::number(tokenCode) + "\"; return; }\n";
            }
            else {
                code += "            else\n";
                code += "                { cout<<\"Error!\"; return; }\n";
            }
            code += "            break;\n";
        }
    }

    code += "        }\n";
    code += "    }\n";
    code += "}\n\n";

    code += "void main()\n";
    code += "{\n";
    code += "    cin.getline(buffer, 255);\n";
    code += "    pos = 0;\n";
    code += "    ch = GetNext();\n";
    code += "    GetToken();\n";
    code += "}\n";

    return code;
}