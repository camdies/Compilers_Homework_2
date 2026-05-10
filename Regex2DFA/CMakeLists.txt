cmake_minimum_required(VERSION 3.16)

# 如果支持,请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Regex2DFA" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== 关键：启用 Qt 的自动 MOC/RCC/UIC =====
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ===== 关键：查找 Qt6 Widgets 模块 =====
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Regex2DFA
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/nfa.h
    src/nfa.cpp
    src/dfa.h
    src/dfa.cpp
    src/utils.h
)
# 关键：为 MSVC 添加必需的编译选项
if(MSVC)
    target_compile_options(Regex2DFA PRIVATE /Zc:__cplusplus /permissive-)
endif()

target_link_libraries(Regex2DFA PRIVATE Qt6::Widgets)