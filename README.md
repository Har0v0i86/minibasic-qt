# MiniBasic · 解释器与可视化执行环境

基于 C++ 和 Qt Widgets 的课程项目，支持编辑、加载和运行带行号的 BASIC 程序，并展示语法树与执行统计。这个仓库整理用于展示表达式解析、面向对象建模和桌面工具交互的实践。

## 功能与实现

- **表达式解析**：词法分词与递归下降解析，支持运算符优先级、括号、一元符号及幂运算。
- **语句执行**：支持 `REM`、`LET`、`PRINT`、`INPUT`、`GOTO`、`IF ... THEN`、`END`。
- **控制流**：语句基类统一执行接口，通过返回码区分顺序执行、跳转与结束。
- **交互与观察**：通过 Qt 界面输入程序和运行命令，查看语法树、变量及执行统计；使用事件循环等待 INPUT 输入。

## 快速开始

依赖：Qt 6 Widgets、支持 C++17 的编译器、CMake 3.16+。选择与 Qt 安装包匹配的编译器（例如 Windows Qt MinGW 套件对应 MinGW）。

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="<Qt 6 kit directory>"
cmake --build build --parallel
```

运行 `build/MiniBasic`；Windows 为 `build/MiniBasic.exe`，多配置生成器可能位于 `build/Debug/`。确保 Qt 的动态库和平台插件可被找到，最直接的方式是在 Qt Creator 中打开 `CMakeLists.txt`、选择 Qt 6 Desktop Kit 后构建运行。仓库也保留原 `MiniBasic.pro`。

在界面中逐行输入下面的程序，再输入 `RUN`：

```basic
10 LET A = 2 + 3 * 4
20 PRINT A
30 END
```

预期输出为 `14`。也可用 `LOAD` 加载本地文本程序；`CLEAR` 清空程序，`HELP` 查看命令，`QUIT` 退出。

## 代码导航

| 文件 | 作用 |
| --- | --- |
| `parser.cpp/.h` | 分词、表达式解析与语句构造 |
| `expression.cpp/.h` | 表达式节点与求值 |
| `statement.cpp/.h` | 语句对象与执行行为 |
| `mainwindow.cpp/.h/.ui` | 界面、命令处理和执行流程 |

## 验证与边界

2026-10-14：在 Windows、Qt 6.9.0、MinGW 13.1、Ninja 环境下实际完成 CMake 配置、编译和链接。未执行 GUI 端到端测试，上面的示例输出为预期行为。

Windows 构建建议使用纯英文路径；本次在中文路径下遇到 Qt moc 输出文件路径错误，改用英文路径后构建成功。

- 这是教学语言解释器，不兼容完整 BASIC 标准。
- 当前未提供独立的自动化回归测试套件；输入校验、异常路径和长时间运行的交互体验仍有完善空间。
- 不把课程框架与全部代码描述为从零原创；该仓库是个人课程项目实现的整理版本。

## 项目说明

作者：[Har0v0i86](https://github.com/Har0v0i86)。源自 SEP 软件工程实践课程项目。本次整理补充 CMake 入口和文档，未重写解释器核心逻辑。未附加开源许可证，公开展示不代表授予额外的复制或再分发许可。
