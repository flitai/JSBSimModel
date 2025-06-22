我们将对`mixr`框架中的`JSBSimDynamics`类进行相同的改造，将其从`mixr`框架中剥离，使其成为一个完全独立的、可以在我们自己应用中调用的C++模块。

我们将遵循与之前完全相同的模式：创建一个独立的C++类，它封装了与JSBSim的所有交互，同时提供清晰的接口用于初始化、控制和数据检索。

-----

### ⚠️ 重要前提：环境配置（与之前相同）

在开始之前，请确保我们的开发环境已经配置好。这部分与我们上次的讨论完全一致。

1.  **安装JSBSim开发库**: 从 [JSBSim的GitHub发布页面](https://github.com/JSBSim-Team/jsbsim/releases) 下载并解压适合我们系统的**开发包 (`-dev-`)**。我们需要记下其 `include` 和 `lib` 文件夹的路径。
2.  **获取JSBSim飞机模型数据**: 从 [JSBSim的主代码仓库](https://github.com/JSBSim-Team/jsbsim) 下载并解压，得到包含 `aircraft`, `engine`, `systems` 等文件夹的数据根目录。
3.  **准备项目文件**: 建议将JSBSim开发库、数据目录和我们的C++源文件放在一个清晰的项目结构中。

**编译指令示例 (Linux/macOS with g++)**:

```bash
# 将 .../jsbsim/install/ 替换为我们的实际路径
g++ main_jsbsim.cpp StandaloneJSBSim.cpp -o JsbSimApp -std=c++17 \
    -I.../jsbsim/install/include \
    -L.../jsbsim/install/lib -lJSBSim
```

-----

### 1\. 基础文件 (复用)

我们将**完全复用**之前创建的三个基础文件，它们的设计是通用的，无需任何修改：

  * `OeBase.hpp`: 提供基础的数学常量、向量类和工具函数。
  * `JSBSimAircraftState.hpp`: 定义了用于存储JSBSim返回的丰富数据的结构体。
  * `main_jsbsim.cpp`: 演示如何初始化、运行和控制独立模型的示例主程序。

前两个文件完全没有变化，`main_jsbsim.cpp`只需略作修改。

-----

### 2\. 独立JSBSim模型 - 头文件 (`StandaloneJSBSim.hpp`)

这个头文件的设计与我们上次为`JSBSimModel`改造的接口几乎完全相同，确保了外部调用的统一性。



### 3\. 独立JSBSim模型 - 实现文件 (`StandaloneJSBSim.cpp`)

这是改造的核心，我们将`mixr::models::jsbsim::JSBSimDynamics`的逻辑移植过来，并替换掉所有对`mixr`框架的依赖。



### 如何使用

之前的`main_jsbsim.cpp`几乎不需要任何改动，只需将实例化的类名从`StandaloneJSBSimModel`改为`StandaloneJSBSim`即可。这表明了我们设计的独立接口的稳定性和可复用性。

```cpp
// 在我们的 main_jsbsim.cpp 中
// ...
#include "StandaloneJSBSim.hpp" // 包含新的头文件

int main() {
    StandaloneJSBSim aircraft; // <--- 只需要修改这里的类名

    // ... 后续所有代码，如init, setInitialConditions, update, getState等调用方式完全相同 ...
}
```

通过以上步骤，我们就成功地将`mixr`框架中的`JSBSimDynamics`改造成了一个功能相同、接口一致的独立C++模块，可以在我们自己的应用中无缝集成和调用。