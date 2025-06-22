// StandaloneJSBSimModel.hpp
#ifndef STANDALONE_JSBSIM_MODEL_HPP
#define STANDALONE_JSBSIM_MODEL_HPP

#include <string>
#include <memory>
#include "JSBSimAircraftState.hpp"

// JSBSim类的正向声明，避免在头文件中包含大型JSBSim头文件
namespace JSBSim {
    class FGFDMExec;
}

class StandaloneJSBSimModel {
public:
    StandaloneJSBSimModel();
    ~StandaloneJSBSimModel();

    // --- 初始化与配置 ---
    bool init(const std::string& jsbsim_root_dir, const std::string& aircraft_model);
    void setInitialConditions(double lat_deg, double lon_deg, double alt_m, double hdg_deg, double speed_kts);
    bool runInitialConditions();

    // --- 核心更新 ---
    void update(double dt);

    // --- 控制指令接口 ---
    void setControlStickRoll(double norm_val);    // -1.0 to 1.0
    void setControlStickPitch(double norm_val);   // -1.0 to 1.0
    void setRudderPedal(double norm_val);       // -1.0 to 1.0
    void setThrottle(int engine_idx, double norm_val); // 0.0 to 1.0
    void setThrottles(double norm_val); //
    void setGearHandle(bool down);

    // --- 获取状态 ---
    const JSBSimAircraftState& getState() const { return m_state; }

private:
    void updateStateFromJSBSim(); // 从JSBSim取回数据的私有函数

    std::unique_ptr<JSBSim::FGFDMExec> fdmex; // 使用智能指针管理JSBSim实例
    JSBSimAircraftState m_state;
};

#endif // STANDALONE_JSBSIM_MODEL_HPP