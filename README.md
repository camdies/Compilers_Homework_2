# Regex2DFA

## 项目简介
Regex2DFA 是一个用于将正则表达式转换为确定有限自动机（DFA）的 C++ 项目，基于 CMake 进行构建。

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
