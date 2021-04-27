#include <genset_whisperpower_ddc/GeneratorState.hpp>

using namespace genset_whisperpower_ddc;

using std::endl;

#define FLAG_OUT(field, flag) \
    if (field & GeneratorState::flag) { \
        io << "    " #flag "\n"; \
    }

#define STATUS_CASE(name) case STATUS_##name: return #name;
std::string statusToString(GeneratorStatus status) {
    switch (status) {
        STATUS_CASE(NONE)
        STATUS_CASE(PRE_GLOW)
        STATUS_CASE(GLOW)
        STATUS_CASE(START)
        STATUS_CASE(SWITCH)
        STATUS_CASE(STOP)
        STATUS_CASE(FAILURE)
        STATUS_CASE(PRESENT)
        STATUS_CASE(FIRST)
        STATUS_CASE(ERROR_BYPASS)
        STATUS_CASE(PRE_ERROR_BYPASS)
        STATUS_CASE(PUMPING)
        STATUS_CASE(RESTART)
        STATUS_CASE(HYDROBOOST)
        STATUS_CASE(UNKNOWN)
    }
    return "UNKNOWN";
}

std::ostream& genset_whisperpower_ddc::operator << (std::ostream& io, GeneratorState const& state) {
    io << "Time: " << state.time << "\n";
    io << "RPM: " << std::dec << state.rpm << "\n";
    io << "Udc start battery: " << std::dec << state.udc_start_battery * 0.01 << " V" << "\n";

    io << "Status flags:\n";
    io << "  Alarms:\n";
    FLAG_OUT(state.status, OVERALL_ALARM);
    FLAG_OUT(state.status, ENGINE_TEMPERATURE_ALARM);
    FLAG_OUT(state.status, PM_VOLTAGE_ALARM);
    FLAG_OUT(state.status, OIL_PRESSURE_ALARM);
    FLAG_OUT(state.status, EXHAUST_TEMPERATURE_ALARM);
    FLAG_OUT(state.status, UAC1_ALARM);
    FLAG_OUT(state.status, IAC1_ALARM);
    FLAG_OUT(state.status, OIL_PRESSURE_HIGH_ALARM);
    FLAG_OUT(state.status, LOW_START_BATTERY_VOLTAGE_ALARM);

    io << "  Model detection:\n";
    FLAG_OUT(state.status, MODEL_DETECTION_50HZ);
    FLAG_OUT(state.status, MODEL_DETECTION_60HZ);
    FLAG_OUT(state.status, MODEL_DETECTION_3_PHASE);
    FLAG_OUT(state.status, MODEL_DETECTION_MOBILE);

    io << "  Others:\n";
    FLAG_OUT(state.status, START_FAILURE);
    FLAG_OUT(state.status, RUN_SIGNAL);
    FLAG_OUT(state.status, START_BY_OPERATION_UNIT);

    io << "generator status: " << statusToString(state.generator_status)  << "\n";
    io << "generator type: " << std::dec << state.generator_type;

    return io;
}