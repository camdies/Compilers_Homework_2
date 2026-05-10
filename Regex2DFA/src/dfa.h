#ifndef DFA_H
#define DFA_H

#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>
#include <QMap>
#include <QList>
#include "nfa.h"

class DFA {
public:
    DFA();
    void clear();

    void fromNFA(const NFA& nfa);
    DFA minimize() const;

    // 生成词法分析C++代码（方法二）
    // symbolRanges: 名称 -> 字符范围 (如 "letter" -> "A-Za-z")
    // placeholderToName: 占位符字符串 -> 名称 (如 "\uE000" -> "letter")
    QString generateLexerCode(const QMap<QString, QString>& symbolRanges,
        const QHash<QString, QString>& placeholderToName,
        const QString& mainRegexName,
        int tokenCode) const;

    int startState;
    QSet<int> endStates;
    int stateCount;
    QList<QString> alphabet;

    QHash<int, QHash<QString, int>> G;
    QHash<int, QSet<int>> mapping;
};

#endif // DFA_H