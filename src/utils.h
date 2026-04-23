#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QSet>
#include <QVector>
#include <QHash>
#include <QMap>

// 运算符优先级
inline int getPriority(QChar ch) {
    if (ch == '*') return 4;
    if (ch == '.') return 2;
    if (ch == '|') return 1;
    if (ch == '(' || ch == ')') return 0;
    return -1;
}

// 判断是否为运算符
inline bool isOperator(QChar ch) {
    return ch == '(' || ch == ')' || ch == '*' || ch == '|' || ch == '.';
}

#endif // UTILS_H