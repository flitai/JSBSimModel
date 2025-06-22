// main_jsbsim.cpp
// 编译: g++ main_jsbsim.cpp StandaloneJSBSimModel.cpp -o JsbSimApp -std=c++17 -I/path/to/jsbsim/include -L/path/to/jsbsim/lib -lJSBSim
// 运行前确保JSBSIM_ROOT_PATH和AIRCRAFT_MODEL是正确的

#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <chrono>
#include "StandaloneJSBSim.hpp" // 和之前不同的头文件

// !!! 用户需要根据自己的环境修改这两个路径 !!!
const std::string JSBSIM_ROOT_PATH = "/path/to/your/jsbsim/data"; // 例如: "/usr/local/share/JSBSim" 或 "./jsbsim"
const std::string AIRCRAFT_MODEL = "c172"; // JSBSim aircraft目录下的飞机名称

int main() {
    StandaloneJSBSim aircraft; // 和之前不同的类名

    // --- 1. 初始化JSBSim ---
    if (!aircraft.init(JSBSIM_ROOT_PATH, AIRCRAFT_MODEL)) {
        return 1;
    }

    // --- 2. 设置初始条件 ---
    // setInitialConditions(lat_deg, lon_deg, alt_m, hdg_deg, speed_kts)
    aircraft.setInitialConditions(34.0, -118.0, 1524, 90, 100); // 洛杉矶附近，5000英尺，向东
    
    if (!aircraft.runInitialConditions()) {
        std::cerr << "JSBSim failed to run initial conditions!" << std::endl;
        return 1;
    }

    // --- 3. 准备日志文件 ---
    std::ofstream outputFile("jsbsim_log.csv");
    outputFile << "Time,Altitude_m,TAS_kts,Mach,G_Load,Roll_deg,Pitch_deg,Yaw_deg,Alpha_deg,Beta_deg\n";

    // --- 4. 仿真循环 ---
    const double dt = 1.0 / 60.0;
    std::cout << "Simulation started. Running for 60 seconds..." << std::endl;

    for (double simTime = 0.0; simTime <= 60.0; simTime += dt) {
        // --- 简单的机动指令 ---
        // 在5-15秒期间，施加一个滚转指令
        if (simTime > 5.0 && simTime < 15.0) {
            aircraft.setControlStickRoll(0.3); // 向右滚转
        } else {
            aircraft.setControlStickRoll(0.0); // 回中
        }
        
        // 保持油门在80%
        aircraft.setThrottles(0.8);

        // --- 更新动力学 ---
        aircraft.update(dt);

        // --- 获取状态并记录 ---
        const JSBSimAircraftState& state = aircraft.getState();
        if (static_cast<int>(simTime * 10) % 10 == 0) { // 每秒打印一次
            std::cout << std::fixed << std::setprecision(2)
                      << "T: " << simTime << "s, Alt: " << state.altitude_sl_m << "m, "
                      << "Vel: " << state.calibrated_airspeed_kts << "kts, "
                      << "G: " << state.g_load << ", Pitch: " << state.pitch_rad * oe_base::angle::R2DCC << "deg"
                      << std::endl;
        }

        outputFile << std::fixed << std::setprecision(4)
                   << simTime << ","
                   << state.altitude_sl_m << ","
                   << state.calibrated_airspeed_kts << ","
                   << state.mach << ","
                   << state.g_load << ","
                   << state.roll_rad * oe_base::angle::R2DCC << ","
                   << state.pitch_rad * oe_base::angle::R2DCC << ","
                   << state.yaw_rad * oe_base::angle::R2DCC << ","
                   << state.alpha_rad * oe_base::angle::R2DCC << ","
                   << state.beta_rad * oe_base::angle::R2DCC << "\n";
        
        // 如果需要实时仿真，可以取消下面的注释
        // std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(dt * 1000)));
    }
    
    outputFile.close();
    std::cout << "Simulation finished. Log file 'jsbsim_log.csv' has been saved." << std::endl;

    return 0;
}