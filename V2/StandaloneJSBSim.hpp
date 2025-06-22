// StandaloneJSBSim.hpp
#ifndef STANDALONE_JSBSIM_HPP
#define STANDALONE_JSBSIM_HPP

#include <string>
#include <memory>
#include "JSBSimAircraftState.hpp"

// JSBSim类的正向声明
namespace JSBSim {
    class FGFDMExec;
}

class StandaloneJSBSim {
public:
    StandaloneJSBSim();
    ~StandaloneJSBSim();

    // --- 初始化与配置 ---
    bool init(const std::string& jsbsim_root_dir, const std::string& aircraft_model, int debug_level = 0);
    void setInitialConditions(double lat_deg, double lon_deg, double alt_m, double hdg_deg, double speed_kts);
    bool runInitialConditions();

    // --- 核心更新 ---
    void update(double dt);

    // --- 控制指令接口 ---
    void setControlStickRoll(double norm_val);      // -1.0 to 1.0
    void setControlStickPitch(double norm_val);     // -1.0 to 1.0
    void setRudderPedal(double norm_val);         // -1.0 to 1.0
    void setThrottle(int engine_idx, double norm_val); // 0.0 to 1.0
    void setThrottles(double norm_val);
    void setGearHandle(bool down);
    void setBrakes(double left, double right);
    void setSpeedBrakes(double norm_val);        // -1.0 (retract), 0.0 (hold), 1.0 (extend)
    void setTrimSwitchRoll(double val);           // -1.0 (left), 0.0 (hold), 1.0 (right)
    void setTrimSwitchPitch(double val);          // -1.0 (down), 0.0 (hold), 1.0 (up)


    // --- 获取状态 ---
    const JSBSimAircraftState& getState() const { return m_state; }

private:
    void updateStateFromJSBSim(); // 从JSBSim取回数据的私有函数
    void updateTrims(double dt);  // 更新配平

    std::unique_ptr<JSBSim::FGFDMExec> fdmex;
    JSBSimAircraftState m_state;

    // 配平相关变量
    double pitchTrimPos{};
    double pitchTrimRate{0.1};
    double pitchTrimSw{};
    double rollTrimPos{};
    double rollTrimRate{0.1};
    double rollTrimSw{};
};

#endif // STANDALONE_JSBSIM_HPP