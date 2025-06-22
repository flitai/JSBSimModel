// JSBSimAircraftState.hpp
#ifndef JSBSIM_AIRCRAFT_STATE_HPP
#define JSBSIM_AIRCRAFT_STATE_HPP

#include "OeBase.hpp" // 复用我们之前的基础工具
#include <vector>

struct PropulsionState {
    double thrust_lbf = 0.0;
    double rpm = 0.0;
    double fuel_flow_pph = 0.0;
    double pla_pct = 0.0; // Power Lever Angle %
};

struct JSBSimAircraftState {
    // --- 运动学核心数据 ---
    oe_base::Vec3d position_ned;      // 位置 (北-东-地), 米
    oe_base::Vec3d velocity_ned;      // 速度 (北-东-地), 米/秒
    oe_base::Vec3d accel_ned;         // 加速度 (北-东-地), 米/秒^2
    double altitude_sl_m = 0.0;       // 海拔高度, 米
    
    // --- 姿态数据 ---
    double roll_rad = 0.0, pitch_rad = 0.0, yaw_rad = 0.0;
    oe_base::Vec3d ang_vel_rps;       // 机体角速度 (p,q,r), 弧度/秒

    // --- 空气动力学数据 ---
    double g_load = 1.0;
    double mach = 0.0;
    double alpha_rad = 0.0;           // 攻角 (Angle of Attack)
    double beta_rad = 0.0;            // 侧滑角 (Sideslip Angle)
    double flight_path_rad = 0.0;     // 飞行路径角 (gamma)
    double calibrated_airspeed_kts = 0.0;

    // --- 系统与重量 ---
    double total_weight_lbs = 0.0;
    double fuel_weight_lbs = 0.0;
    bool on_ground = false;

    // --- 发动机数据 ---
    int num_engines = 0;
    std::vector<PropulsionState> propulsion;
};

#endif // JSBSIM_AIRCRAFT_STATE_HPP