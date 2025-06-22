这是一个**仅依赖于标准C++库和JSBSim库**的独立应用程序，能够初始化和配置JSBSim，将控制意图转换给JSBSim，调用其运算，并返回精确结果。



### ⚠️ 重要前提：环境配置

在编译和运行下面的代码之前，**必须**在系统上**安装JSBSim开发库**，并获取其飞机模型文件。

1.  **安装JSBSim**: 请从 [JSBSim的GitHub发布页面](https://github.com/JSBSim-Team/jsbsim/releases) 下载并安装适合您操作系统的预编译版本，或者从源码编译。安装后，您需要知道其**头文件 (`include`)** 和 **库文件 (`lib`)** 的路径。
2.  **获取飞机模型**: 从JSBSim的源码或数据包中，需要一个包含 `aircraft`, `engine`, `systems` 等文件夹的JSBSim根目录。

**编译指令示例 (Linux/macOS with g++)**:

```bash
g++ main_jsbsim.cpp StandaloneJSBSimModel.cpp -o JsbSimApp -std=c++17 \
    -I/path/to/your/jsbsim/install/include \
    -L/path/to/your/jsbsim/install/lib -lJSBSim
```

我们需要将 `/path/to/your/jsbsim/install/` 替换为实际的JSBSim安装路径。

-----

### 1\. 扩展的状态结构体 (`JSBSimAircraftState.hpp`)

为了容纳JSBSim提供的丰富数据，我们需要一个比之前更详细的状态结构体。



### 2\. 独立JSBSim模型 - 头文件 (`StandaloneJSBSimModel.hpp`)

这是改造后模型的公共接口，负责与JSBSim引擎直接交互。

-----

### 3\. 独立JSBSim模型 - 实现文件 (`StandaloneJSBSimModel.cpp`)

这是模型核心逻辑的实现，负责调用JSBSim API。



### 4\. 示例主程序 (`main_jsbsim.cpp`)

这个主程序演示了如何实例化、配置、运行并控制`StandaloneJSBSimModel`。

