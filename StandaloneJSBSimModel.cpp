// StandaloneJSBSimModel.cpp
#include "StandaloneJSBSimModel.hpp"

// 引入所有需要的JSBSim头文件
#include <JSBSim/FGFDMExec.h>
#include <JSBSim/models/FGAuxiliary.h>
#include <JSBSim/models/FGPropulsion.h>
#include <JSBSim/models/FGFCS.h>
#include <JSBSim/models/FGGroundReactions.h>
#include <JSBSim/models/FGPropagate.h>
#include <JSBSim/models/FGAccelerations.h>
#include <JSBSim/initialization/FGInitialCondition.h>
#include <JSBSim/models/propulsion/FGEngine.h>
#include <JSBSim/models/propulsion/FGThruster.h>

#include <iostream>

StandaloneJSBSimModel::StandaloneJSBSimModel() = default;
StandaloneJSBSimModel::~StandaloneJSBSimModel() = default;

bool StandaloneJSBSimModel::init(const std::string& jsbsim_root_dir, const std::string& aircraft_model) {
    fdmex = std::make_unique<JSBSim::FGFDMExec>();
    if (!fdmex) return false;

    std::cout << "Using JSBSim version " << fdmex->GetVersion() << std::endl;

    fdmex->SetRootDir(SGPath(jsbsim_root_dir));
    
    std::cout << "Loading aircraft model: " << aircraft_model << std::endl;
    if (!fdmex->LoadModel(aircraft_model)) {
        std::cerr << "Failed to load JSBSim model!" << std::endl;
        fdmex.reset();
        return false;
    }
    
    // 初始化状态结构体
    m_state.num_engines = fdmex->GetPropulsion()->GetNumEngines();
    m_state.propulsion.resize(m_state.num_engines);

    return true;
}

void StandaloneJSBSimModel::setInitialConditions(double lat_deg, double lon_deg, double alt_m, double hdg_deg, double speed_kts) {
    if (!fdmex) return;
    auto ic = fdmex->GetIC();
    ic->SetLatitudeDegIC(lat_deg);
    ic->SetLongitudeDegIC(lon_deg);
    ic->SetAltitudeASLFtIC(alt_m * 3.28084);
    ic->SetPsiDegIC(hdg_deg);
    ic->SetVtrueKtsIC(speed_kts);
}

bool StandaloneJSBSimModel::runInitialConditions() {
    if (!fdmex) return false;
    
    // 启动引擎
    auto propulsion = fdmex->GetPropulsion();
    for (int i = 0; i < propulsion->GetNumEngines(); ++i) {
        propulsion->GetEngine(i)->SetRunning(true);
    }
    
    bool result = fdmex->RunIC();
    if (result) {
        updateStateFromJSBSim(); // 运行IC后立即更新一次状态
    }
    return result;
}

void StandaloneJSBSimModel::update(double dt) {
    if (!fdmex) return;
    fdmex->Setdt(dt);
    fdmex->Run();
    updateStateFromJSBSim();
}

void StandaloneJSBSimModel::updateStateFromJSBSim() {
    if (!fdmex) return;
    auto prop = fdmex->GetPropagate();
    auto aux = fdmex->GetAuxiliary();
    auto accel = fdmex->GetAccelerations();
    auto fcs = fdmex->GetFCS();
    auto ground = fdmex->GetGroundReactions();
    auto propulsion = fdmex->GetPropulsion();

    const double FT2M = 1.0 / 3.28084;
    
    // --- 运动学 ---
    m_state.position_ned.set(prop->GetLocation().GetLatitudeDeg(), prop->GetLocation().GetLongitudeDeg(), prop->GetAltitudeASLmeters());
    m_state.altitude_sl_m = prop->GetAltitudeASLmeters();
    m_state.velocity_ned.set(prop->GetVel(JSBSim::FGJSBBase::eNorth) * FT2M, prop->GetVel(JSBSim::FGJSBBase::eEast) * FT2M, prop->GetVel(JSBSim::FGJSBBase::eDown) * FT2M);
    m_state.accel_ned.set(accel->GetNedAccel(1), accel->GetNedAccel(2), accel->GetNedAccel(3));
    
    // --- 姿态 ---
    m_state.roll_rad = prop->GetEuler(JSBSim::FGJSBBase::ePhi);
    m_state.pitch_rad = prop->GetEuler(JSBSim::FGJSBBase::eTht);
    m_state.yaw_rad = prop->GetEuler(JSBSim::FGJSBBase::ePsi);
    m_state.ang_vel_rps.set(prop->GetPQR(JSBSim::FGJSBBase::eP), prop->GetPQR(JSBSim::FGJSBBase::eQ), prop->GetPQR(JSBSim::FGJSBBase::eR));

    // --- 空气动力学 ---
    m_state.g_load = aux->GetNlf();
    m_state.mach = aux->GetMach();
    m_state.alpha_rad = aux->Getalpha();
    m_state.beta_rad = aux->Getbeta();
    m_state.flight_path_rad = aux->GetGamma();
    m_state.calibrated_airspeed_kts = aux->GetVcalibratedKTS();

    // --- 系统 ---
    m_state.total_weight_lbs = fdmex->GetMassBalance()->GetWeight();
    m_state.on_ground = ground->GetWOW();

    // --- 发动机 ---
    m_state.fuel_weight_lbs = propulsion->GetFuelWt();
    for (int i = 0; i < m_state.num_engines; ++i) {
        auto engine = propulsion->GetEngine(i);
        m_state.propulsion[i].thrust_lbf = engine->GetThruster()->GetThrust();
        m_state.propulsion[i].rpm = engine->getRPM();
        m_state.propulsion[i].fuel_flow_pph = engine->getFuelFlow_pph();
        m_state.propulsion[i].pla_pct = (fcs->GetThrottlePos(i) - engine->GetThrottleMin()) / (engine->GetThrottleMax() - engine->GetThrottleMin()) * 100.0;
    }
}

// --- 控制指令实现 ---
void StandaloneJSBSimModel::setControlStickRoll(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDaCmd(norm_val);
}

void StandaloneJSBSimModel::setControlStickPitch(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDeCmd(-norm_val); // JSBSim升降舵方向与通用习惯相反
}

void StandaloneJSBSimModel::setRudderPedal(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDrCmd(-norm_val); // JSBSim方向舵方向与通用习惯相反
}

void StandaloneJSBSimModel::setThrottle(int engine_idx, double norm_val) {
    if (fdmex && engine_idx < m_state.num_engines) fdmex->GetFCS()->SetThrottleCmd(engine_idx, norm_val);
}

void StandaloneJSBSimModel::setThrottles(double norm_val) {
    if (fdmex) {
        for (int i = 0; i < m_state.num_engines; ++i) {
            fdmex->GetFCS()->SetThrottleCmd(i, norm_val);
        }
    }
}

void StandaloneJSBSimModel::setGearHandle(bool down) {
    if (fdmex) fdmex->GetFCS()->SetGearCmd(down ? 1.0 : 0.0);
}