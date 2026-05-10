# Regex2DFA

## 项目简介
Regex2DFA 是一个基于 C++ 和 Qt6 开发的可视化工具。它实现了编译器理论中的经典算法，让你能够直观地看到一个正则表达式如何通过汤普森构造法（Thompson’s construction）构建 NFA，再通过子集构造法（Subset construction）将 NFA 转换为 DFA。

## 核心功能
- 正则表达式解析：输入一个正则表达式字符串。
- NFA 转换：将正则表达式转换为一个等价的 NFA状态转换表。
- DFA 转换：将生成的 NFA 转换为等价的 DFA状态转换表。
- DFA 最小化：对生成的 DFA 进行状态最小化，以获得最高效的自动机。
- 词法分析代码生成：将最小化后的 DFA 导出为一个完整的 C++ 类，该类可以用于匹配符合原始正则表达式的字符串。
- 结果可视化：以文本形式清晰地展示 NFA 和 DFA 的状态、转移和接受状态。

## 安装
- 需要 CMake（建议 3.20+）
- 需要支持 C++17 的编译器（如 GCC/Clang/MSVC）
- 建议使用msvs2022编译运行此项目，VS 会自动配置 CMake 项目。如果找不到 Qt6，你可能需要在 CMakeSettings.json 中手动指定 Qt6 的路径，将CMAKE_PREFIX_PATH请务必修改成你的Qt目录的适用msvc2022_64的对应文件夹目录

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

## 快速入门
- 启动 Regex2DFA 应用程序。
- 在顶部的 “输入正则表达式” 文本框中，输入一个表达式，例如 (a|b)*c。
- 点击 “构建自动机 按钮。
- 在下方的结果区域，你将看到三个选项卡，分别显示了 NFA、DFA 和 词法分析代码 的详细信息。
- 切换到 词法分析代码 选项卡，将生成的代码复制并保存为cpp文件，以便在你的项目中使用。
