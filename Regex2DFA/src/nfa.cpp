#include "nfa.h"
#include "utils.h"
#include <QStack>
#include <QQueue>
#include <algorithm>

NFA::NFA() : startState(-1), endState(-1), stateCount(0) {
}

void NFA::clear() {
    edges.clear();
    alphabet.clear();
    orderedAlphabet.clear();
    startState = -1;
    endState = -1;
    stateCount = 0;
}

int NFA::newState() {
    return stateCount++;
}

void NFA::fromPostfix(const QString& postfix) {
    clear();
    QStack<QPair<int, int>> stk;

    for (int i = 0; i < postfix.size(); i++) {
        QChar ch = postfix[i];

        if (ch == '\\') {
            if (i + 1 < postfix.size()) {
                i++;
                QChar actual = postfix[i];
                int s = newState();
                int e = newState();
                edges.append({ s, e, QString(actual) });
                alphabet.insert(QString(actual));
                stk.push({ s, e });
            }
        }
        else if (ch == '.') {
            if (stk.size() >= 2) {
                auto second = stk.top(); stk.pop();
                auto first = stk.top(); stk.pop();
                edges.append({ first.second, second.first, "epsilon" });
                stk.push({ first.first, second.second });
            }
        }
        else if (ch == '|') {
            if (stk.size() >= 2) {
                auto second = stk.top(); stk.pop();
                auto first = stk.top(); stk.pop();
                int s = newState();
                int e = newState();
                edges.append({ s, first.first, "epsilon" });
                edges.append({ s, second.first, "epsilon" });
                edges.append({ first.second, e, "epsilon" });
                edges.append({ second.second, e, "epsilon" });
                stk.push({ s, e });
            }
        }
        else if (ch == '*') {
            if (!stk.empty()) {
                auto item = stk.top(); stk.pop();
                int s = newState();
                int e = newState();
                edges.append({ s, item.first, "epsilon" });
                edges.append({ s, e, "epsilon" });
                edges.append({ item.second, item.first, "epsilon" });
                edges.append({ item.second, e, "epsilon" });
                stk.push({ s, e });
            }
        }
        else if (ch == '#') {
            int s = newState();
            int e = newState();
            edges.append({ s, e, "epsilon" });
            stk.push({ s, e });
        }
        else {
            int s = newState();
            int e = newState();
            edges.append({ s, e, QString(ch) });
            alphabet.insert(QString(ch));
            stk.push({ s, e });
        }
    }

    if (!stk.empty()) {
        auto result = stk.top();
        startState = result.first;
        endState = result.second;
    }

    // ===== 关键修复：BFS重编号状态，使编号从起始态开始按拓扑顺序递增 =====
    renumberStatesBFS();

    QList<QString> sortedAlpha = alphabet.values();
    std::sort(sortedAlpha.begin(), sortedAlpha.end());
    orderedAlphabet = sortedAlpha;
}

QSet<int> NFA::epsilonClosure(const QSet<int>& states) const {
    QSet<int> closure = states;
    QStack<int> worklist;
    for (int s : states) {
        worklist.push(s);
    }

    while (!worklist.empty()) {
        int current = worklist.top();
        worklist.pop();
        for (const auto& edge : edges) {
            if (edge.from == current && edge.symbol == "epsilon") {
                if (!closure.contains(edge.to)) {
                    closure.insert(edge.to);
                    worklist.push(edge.to);
                }
            }
        }
    }
    return closure;
}

QSet<int> NFA::move(const QSet<int>& states, const QString& symbol) const {
    QSet<int> result;
    for (int s : states) {
        for (const auto& edge : edges) {
            if (edge.from == s && edge.symbol == symbol) {
                result.insert(edge.to);
            }
        }
    }
    return result;
}

void NFA::renumberStatesBFS() {
    if (stateCount == 0 || startState < 0) return;

    QHash<int, int> oldToNew;
    QQueue<int> queue;

    // 预留：起始态 = 0，终态 = stateCount - 1
    int lastId = stateCount - 1;

    // 先固定起始态和终态的编号
    oldToNew[startState] = 0;
    if (startState != endState) {
        oldToNew[endState] = lastId;
    }

    // BFS从起始状态开始，为其余状态分配中间编号
    int newId = 1; // 从1开始（0已给起始态）
    queue.enqueue(startState);
    QSet<int> visited;
    visited.insert(startState);

    while (!queue.empty()) {
        int current = queue.dequeue();

        // 按边的添加顺序遍历后继状态
        for (const auto& edge : edges) {
            if (edge.from == current && !visited.contains(edge.to)) {
                visited.insert(edge.to);

                // 终态已经预分配了最大编号，跳过
                if (edge.to == endState) {
                    queue.enqueue(edge.to);
                    continue;
                }

                // 跳过已被终态占用的编号
                if (newId == lastId) newId++;

                oldToNew[edge.to] = newId++;
                queue.enqueue(edge.to);
            }
        }
    }

    // 处理未被BFS访问到的孤立状态（理论上不应存在）
    for (int i = 0; i < stateCount; i++) {
        if (!oldToNew.contains(i)) {
            if (newId == lastId) newId++;
            oldToNew[i] = newId++;
        }
    }

    // 应用新编号到所有边
    for (auto& edge : edges) {
        edge.from = oldToNew[edge.from];
        edge.to = oldToNew[edge.to];
    }

    // 更新起始态和终态
    startState = 0;           // 最小编号（显示为1）
    endState = lastId;        // 最大编号（显示为stateCount）
}

/*
void NFA::renumberStatesBFS() {
    if (stateCount == 0 || startState < 0) return;

    // BFS从起始状态开始，按照访问顺序分配新编号
    QHash<int, int> oldToNew;
    QQueue<int> queue;
    int newId = 0;

    queue.enqueue(startState);
    oldToNew[startState] = newId++;

    while (!queue.empty()) {
        int current = queue.dequeue();

        // 收集当前状态的所有后继状态，按照边的添加顺序处理
        for (const auto& edge : edges) {
            if (edge.from == current) {
                if (!oldToNew.contains(edge.to)) {
                    oldToNew[edge.to] = newId++;
                    queue.enqueue(edge.to);
                }
            }
        }
    }

    // 处理可能未被BFS访问到的孤立状态（理论上不应该存在）
    for (int i = 0; i < stateCount; i++) {
        if (!oldToNew.contains(i)) {
            oldToNew[i] = newId++;
        }
    }

    // 应用新编号到所有边
    for (auto& edge : edges) {
        edge.from = oldToNew[edge.from];
        edge.to = oldToNew[edge.to];
    }

    // 更新起始态和终态
    startState = oldToNew[startState];
    endState = oldToNew[endState];
}
*/