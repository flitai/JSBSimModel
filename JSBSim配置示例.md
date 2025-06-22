# JSBSim配置示例

为了确保能顺利地将我们创建的独立 `StandaloneJSBSimModel` C++代码与JSBSim引擎完美结合，以下为配置步骤、注意事项，并以JSBSim中典型的**小型固定翼飞机Cessna 172 (c172)** 为例进行说明。

-----

### 第一步：获取并安装JSBSim开发库

这是所有工作的基础。我们需要获取包含头文件（用于编译）和库文件（用于链接）的JSBSim开发包。

**推荐方式：使用预编译的开发包 (最简单)**

1.  访问JSBSim的官方GitHub发布页面：[https://github.com/JSBSim-Team/jsbsim/releases](https://github.com/JSBSim-Team/jsbsim/releases)
2.  寻找最新版本的Assets（资源）列表。
3.  根据我们的操作系统，下载名称中带有 **`-dev-`** 的压缩包。例如：
      * **Windows**: `jsbsim-v1.1.11-dev-windows-x64.zip`
      * **Linux**: `jsbsim-v1.1.11-dev-linux-x64.tar.gz`
      * **macOS**: `jsbsim-v1.1.11-dev-macos-x64.tar.gz`
4.  将下载的压缩包解压到一个我们容易找到的位置。解压后，我们会得到一个类似这样的文件夹结构，其中`include`**和**`lib`（或`bin`）是我们最需要关注的：
    ```
    jsbsim-install/
    |-- include/
    |   |-- JSBSim/
    |       |-- FGFDMExec.h
    |       |-- ... (所有 .h 头文件)
    |-- lib/
    |   |-- JSBSim.lib (Windows) 或 libJSBSim.so (Linux) 或 libJSBSim.dylib (macOS)
    |-- bin/
        |-- JSBSim.dll (Windows动态链接库)
    ```

**(备选方式：从源码编译)**
如果我们需要特定版本或有特殊需求，可以从源码编译。这需要我们安装`CMake`和C++编译器（如Visual Studio, g++, Clang）。基本流程是：
`git clone https://github.com/JSBSim-Team/jsbsim.git`
`cd jsbsim`
`mkdir build && cd build`
`cmake ..`
`make` (或在Visual Studio中构建)
`make install` (将文件安装到指定目录)

-----

### 第二步：获取JSBSim飞机模型数据

JSBSim引擎需要大量的XML文件来定义飞机、发动机、系统等。

1.  访问JSBSim的主代码仓库：[https://github.com/JSBSim-Team/jsbsim](https://github.com/JSBSim-Team/jsbsim)
2.  点击绿色的 "Code" 按钮，然后选择 "Download ZIP"。
3.  解压下载的 `jsbsim-main.zip` 文件。
4.  在解压后的文件夹中，我们会找到 `aircraft`, `engine`, `systems`, `scripts` 等目录。将这个包含所有这些目录的**根文件夹**整体复制到我们的项目中。我们可以将其重命名为 `jsbsim-data` 以便识别。

-----

### 第三步：组织我们的项目文件结构

一个清晰的文件结构会让后续的编译和运行变得简单。推荐的结构如下：

```
MyJsbSimProject/
|
|-- jsbsim-install/              <-- 第一步中解压的开发库
|   |-- include/
|   |-- lib/
|   |-- bin/
|
|-- jsbsim-data/                 <-- 第二步中下载的数据根目录
|   |-- aircraft/
|   |-- engine/
|   |-- systems/
|   |-- ...
|
|-- main_jsbsim.cpp              <-- 我们的主程序
|-- StandaloneJSBSimModel.hpp
|-- StandaloneJSBSimModel.cpp
|-- JSBSimAircraftState.hpp
|-- OeBase.hpp
```

-----

### 第四步：修改代码并编译 (以Cessna 172为例)

现在，我们将使用**Cessna 172 (c172)** 作为我们的目标飞机，并修改`main_jsbsim.cpp`来调用它。

1.  **修改 `main_jsbsim.cpp`**:
    打开 `main_jsbsim.cpp` 文件，找到文件顶部的两个常量，并修改为指向我们的文件结构和目标飞机：

    ```cpp
    // main_jsbsim.cpp
    
    // !!! 用户需要根据自己的环境修改这两个路径 !!!
    const std::string JSBSIM_ROOT_PATH = "./jsbsim-data"; // 指向我们在第二步中准备好的数据根目录
    const std::string AIRCRAFT_MODEL = "c172"; // 目标飞机是c172 (对应aircraft/c172.xml)
    ```

2.  **编译我们的应用程序**:
    这是最关键的一步，我们需要告诉编译器去哪里找JSBSim的头文件和库文件。

      * **在Linux / macOS (使用g++)**:
        打开终端，确保我们在 `MyJsbSimProject` 目录下，然后运行以下命令：

        ```bash
        g++ main_jsbsim.cpp StandaloneJSBSimModel.cpp -o JsbSimApp -std=c++17 \
        -I./jsbsim-install/include \
        -L./jsbsim-install/lib -lJSBSim
        ```

          * `-I./jsbsim-install/include`: 指定JSBSim头文件的搜索路径。
          * `-L./jsbsim-install/lib`: 指定JSBSim库文件的搜索路径。
          * `-lJSBSim`: 指示链接器链接到`JSBSim`库。

      * **在Windows (使用Visual Studio)**:
        在IDE中操作比命令行更方便。

        1.  创建一个新的C++空项目。
        2.  将所有 `.hpp` 和 `.cpp` 文件添加到项目中。
        3.  打开 **项目属性 (Project Properties)**:
              * 在 **C/C++ -\> General -\> Additional Include Directories** 中，添加 `jsbsim-install/include` 的绝对路径。
              * 在 **Linker -\> General -\> Additional Library Directories** 中，添加 `jsbsim-install/lib` 的绝对路径。
              * 在 **Linker -\> Input -\> Additional Dependencies** 中，添加 `JSBSim.lib`。
        4.  确保项目配置为 `Release` 和 `x64` (如果我们的库是64位的)。
        5.  构建项目。

-----

### 第五步：运行与验证

1.  **准备动态库 (仅限Windows)**:
    在运行编译好的 `JsbSimApp.exe` 之前，需要将 `jsbsim-install/bin/JSBSim.dll` 文件复制到与我们的 `.exe` 文件相同的目录下。

2.  **运行程序**:

      * **Linux/macOS**: `./JsbSimApp`
      * **Windows**: `JsbSimApp.exe`

3.  **验证**:
    如果一切顺利，我们将在控制台看到仿真开始运行并每秒打印一次状态数据。同时，项目目录下会生成一个 `jsbsim_log.csv` 文件。我们可以打开这个CSV文件，看到飞机从100节的速度开始，在施加滚转指令后，其滚转角 (`Roll_deg`) 发生变化，姿态和高度也随之改变，这证明我们的独立应用已成功驱动JSBSim引擎。

### 注意事项

  * **库与头文件版本匹配**: 确保我们使用的JSBSim库文件 (`.lib`, `.so`) 和头文件 (`.h`) 来自同一个JSBSim版本，否则可能导致链接错误或运行时崩溃。
  * **动态库路径**: 在Linux上，如果库没有安装到标准路径，我们可能需要在运行前设置环境变量：`export LD_LIBRARY_PATH=./jsbsim-install/lib:$LD_LIBRARY_PATH`。在Windows上，确保DLL文件在正确的位置是必须的。
  * **JSBSim数据根目录路径**: 代码中 `JSBSIM_ROOT_PATH` 必须准确指向包含`aircraft`, `engine`等子目录的文件夹，否则JSBSim会因为找不到XML文件而加载失败。
  * **飞机模型名称**: 代码中 `AIRCRAFT_MODEL` 的值（如`"c172"`）必须与`jsbsim-data/aircraft/`目录下的飞机描述文件名（`c172.xml`）相对应（不含`.xml`后缀）。
  * **C++标准**: 我们的代码使用了C++17标准，请确保我们的编译器支持并已启用该标准。