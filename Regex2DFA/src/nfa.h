#ifndef NFA_H
#define NFA_H

#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QList>
#include <QPair>

// NFA的一条边
struct NFAEdge {
    int from;
    int to;
    QString symbol; // "epsilon" 表示ε转移，其他为输入符号
};

class NFA {
public:
    NFA();
    void clear();

    // 从后缀表达式构造NFA（Thompson构造法）
    void fromPostfix(const QString& postfix);

    // 计算epsilon闭包
    QSet<int> epsilonClosure(const QSet<int>& states) const;

    // 计算状态集合经过symbol转移后到达的状态集合
    QSet<int> move(const QSet<int>& states, const QString& symbol) const;

    // 成员变量
    QVector<NFAEdge> edges;        // 所有边
    QSet<QString> alphabet;        // 输入字母表(不含epsilon)
    QList<QString> orderedAlphabet; // 有序字母表，用于显示
    int startState;
    int endState;
    int stateCount;

private:
    int newState();
    void renumberStatesBFS();
};

#endif // NFA_H