# Regex2DFA

## 项目简介
Regex2DFA 是一个基于 C++ 和 Qt6 开发的可视化工具。它实现了编译器理论中的经典算法，让你能够直观地看到一个正则表达式如何通过汤普森构造法（Thompson’s construction）构建 NFA，再通过子集构造法（Subset construction）将 NFA 转换为 DFA。
## 安装
- 需要 CMake（建议 3.20+）
- 需要支持 C++17 的编译器（如 GCC/Clang/MSVC）

## 构建步骤
```bash
# 1. 获取源码
git clone https://github.com/camdies/Regex2DFA.git
cd Regex2DFA

# 2. 生成构建目录
cmake -S . -B build

# 3. 编译
cmake --build build
```
