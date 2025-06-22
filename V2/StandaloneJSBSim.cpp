// StandaloneJSBSim.cpp
#include "StandaloneJSBSim.hpp"

// 引入所有需要的JSBSim头文件
#include <JSBSim/FGFDMExec.h>
#include <JSBSim/models/FGAuxiliary.h>
#include <JSBSim/models/FGPropulsion.h>
#include <JSBSim/models/FGFCS.h>
#include <JSBSim/models/FGGroundReactions.h>
#include <JSBSim/models/FGPropagate.h>
#include <JSBSim/models/FGAccelerations.h>
#include <JSBSim/models/FGMassBalance.h>
#include <JSBSim/initialization/FGInitialCondition.h>
#include <JSBSim/models/propulsion/FGEngine.h>
#include <JSBSim/models/propulsion/FGThruster.h>
#include <JSBSim/simgear/misc/sg_path.hxx>

#include <iostream>

StandaloneJSBSim::StandaloneJSBSim() = default;
StandaloneJSBSim::~StandaloneJSBSim() = default;

bool StandaloneJSBSim::init(const std::string& jsbsim_root_dir, const std::string& aircraft_model, int debug_level) {
    fdmex = std::make_unique<JSBSim::FGFDMExec>();
    if (!fdmex) return false;

    std::cout << "Using JSBSim version " << fdmex->GetVersion() << std::endl;

    // 使用SGPath来处理UTF-8路径，这与源文件逻辑保持一致
    fdmex->SetRootDir(SGPath::fromString(jsbsim_root_dir));
    fdmex->SetAircraftPath(SGPath::from_string(jsbsim_root_dir + "/aircraft"));
    fdmex->SetEnginePath(SGPath::from_string(jsbsim_root_dir + "/engine"));
    fdmex->SetSystemsPath(SGPath::from_string(jsbsim_root_dir + "/systems"));
    fdmex->SetDebugLevel(debug_level);
    
    std::cout << "Loading aircraft model: " << aircraft_model << std::endl;
    if (!fdmex->LoadModel(aircraft_model)) {
        std::cerr << "Failed to load JSBSim model!" << std::endl;
        fdmex.reset();
        return false;
    }
    
    m_state.num_engines = fdmex->GetPropulsion()->GetNumEngines();
    m_state.propulsion.resize(m_state.num_engines);

    return true;
}

void StandaloneJSBSim::setInitialConditions(double lat_deg, double lon_deg, double alt_m, double hdg_deg, double speed_kts) {
    if (!fdmex) return;
    auto ic = fdmex->GetIC();
    ic->SetLatitudeDegIC(lat_deg);
    ic->SetLongitudeDegIC(lon_deg);
    ic->SetAltitudeASLFtIC(alt_m / oe_base::FT2M);
    ic->SetPsiDegIC(hdg_deg);
    ic->SetVtrueKtsIC(speed_kts);
}

bool StandaloneJSBSim::runInitialConditions() {
    if (!fdmex) return false;
    
    // 启动引擎
    auto propulsion = fdmex->GetPropulsion();
    auto fcs = fdmex->GetFCS();
    for (int i = 0; i < propulsion->GetNumEngines(); ++i) {
        propulsion->GetEngine(i)->SetRunning(true);
        fcs->SetThrottleCmd(i, 1.0); // Set initial throttle to full
    }
    propulsion->InitRunning(-1);
    
    bool result = fdmex->RunIC();
    if (result) {
        updateStateFromJSBSim();
    }
    return result;
}

void StandaloneJSBSim::update(double dt) {
    if (!fdmex) return;
    updateTrims(dt);
    fdmex->Setdt(dt);
    if (!fdmex->Run()) {
        std::cerr << "JSBSim::Run() failed!" << std::endl;
    }
    updateStateFromJSBSim();
}

void StandaloneJSBSim::updateStateFromJSBSim() {
    if (!fdmex) return;
    auto prop = fdmex->GetPropagate();
    auto aux = fdmex->GetAuxiliary();
    auto accel = fdmex->GetAccelerations();
    auto fcs = fdmex->GetFCS();
    auto ground = fdmex->GetGroundReactions();
    auto propulsion = fdmex->GetPropulsion();
    auto mass = fdmex->GetMassBalance();

    const double R2D = oe_base::angle::R2DCC;

    // --- 运动学 ---
    m_state.position_ned.set(prop->GetLocation().GetLatitudeDeg(), prop->GetLocation().GetLongitudeDeg(), -prop->GetAltitudeASLmeters());
    m_state.altitude_sl_m = prop->GetAltitudeASLmeters();
    m_state.velocity_ned.set(prop->GetVel(JSBSim::FGJSBBase::eNorth) * oe_base::FT2M, prop->GetVel(JSBSim::FGJSBBase::eEast) * oe_base::FT2M, prop->GetVel(JSBSim::FGJSBBase::eDown) * oe_base::FT2M);
    
    const JSBSim::FGMatrix33& Tb2l{prop->GetTb2l()};
    const JSBSim::FGColumnVector3& vUVWdot{accel->GetUVWdot()};
    JSBSim::FGColumnVector3 vVeldot{Tb2l * vUVWdot};
    m_state.accel_ned.set(vVeldot(1) * oe_base::FT2M, vVeldot(2) * oe_base::FT2M, vVeldot(3) * oe_base::FT2M);
    
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
    m_state.total_weight_lbs = mass->GetWeight();
    m_state.on_ground = ground->GetWOW();

    // --- 发动机 ---
    m_state.fuel_weight_lbs = propulsion->GetFuelWt();
    for (int i = 0; i < m_state.num_engines; ++i) {
        auto engine = propulsion->GetEngine(i);
        m_state.propulsion[i].thrust_lbf = engine->GetThruster()->GetThrust();
        m_state.propulsion[i].rpm = engine->getRPM();
        m_state.propulsion[i].fuel_flow_pph = engine->getFuelFlow_pph();
        const double tmax = engine->GetThrottleMax();
        const double tmin = engine->GetThrottleMin();
        if (tmax > tmin) {
            m_state.propulsion[i].pla_pct = (fcs->GetThrottlePos(i) - tmin) / (tmax - tmin) * 100.0;
        }
    }
}

// --- 控制指令实现 ---
void StandaloneJSBSim::updateTrims(double dt) {
    if (!fdmex) return;
    auto fcs = fdmex->GetFCS();
    
    pitchTrimPos += pitchTrimRate * pitchTrimSw * dt;
    pitchTrimPos = std::max(-1.0, std::min(1.0, pitchTrimPos));
    fcs->SetPitchTrimCmd(pitchTrimPos);

    rollTrimPos += rollTrimRate * rollTrimSw * dt;
    rollTrimPos = std::max(-1.0, std::min(1.0, rollTrimPos));
    fcs->SetRollTrimCmd(rollTrimPos);
}

void StandaloneJSBSim::setControlStickRoll(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDaCmd(norm_val);
}

void StandaloneJSBSim::setControlStickPitch(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDeCmd(-norm_val);
}

void StandaloneJSBSim::setRudderPedal(double norm_val) {
    if (fdmex) fdmex->GetFCS()->SetDrCmd(-norm_val);
}

void StandaloneJSBSim::setThrottle(int engine_idx, double norm_val) {
    if (fdmex && engine_idx < m_state.num_engines) fdmex->GetFCS()->SetThrottleCmd(engine_idx, norm_val);
}

void StandaloneJSBSim::setThrottles(double norm_val) {
    if (fdmex) {
        for (int i = 0; i < m_state.num_engines; ++i) {
            fdmex->GetFCS()->SetThrottleCmd(i, norm_val);
        }
    }
}

void StandaloneJSBSim::setGearHandle(bool down) {
    if (fdmex) fdmex->GetFCS()->SetGearCmd(down ? 1.0 : 0.0);
}

void StandaloneJSBSim::setBrakes(double left, double right) {
    if (fdmex) {
        auto fcs = fdmex->GetFCS();
        fcs->SetLBrake(left);
        fcs->SetRBrake(right);
    }
}

void StandaloneJSBSim::setSpeedBrakes(double norm_val) {
    if (fdmex) {
        if (norm_val > 0) fdmex->GetFCS()->SetDsbCmd(1.0);
        else if (norm_val < 0) fdmex->GetFCS()->SetDsbCmd(0.0);
    }
}

void StandaloneJSBSim::setTrimSwitchRoll(double val) {
    rollTrimSw = val;
}

void StandaloneJSBSim::setTrimSwitchPitch(double val) {
    pitchTrimSw = -val; // 和源文件逻辑保持一致
}