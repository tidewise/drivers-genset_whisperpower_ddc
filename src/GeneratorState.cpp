#include <genset_whisperpower_ddc/GeneratorState.hpp>

using namespace genset_whisperpower_ddc;

using std::endl;

#define FLAG_OUT(field, flag)                                                            \
    if (field & GeneratorState::flag) {                                                  \
        io << "    " #flag "\n";                                                         \
    }

#define STATUS_CASE(name)                                                                \
    case STATUS_##name:                                                                  \
        return #name;
std::string statusToString(GeneratorStatus status)
{
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

std::ostream& genset_whisperpower_ddc::operator<<(std::ostream& io,
    GeneratorState const& state)
{
    io << "Time: " << state.time << "\n";
    io << "Rotation speed: " << std::dec << state.rotation_speed << "rad/s\n";
    io << "Start battery voltage: " << std::dec << state.start_battery_voltage << " V\n";

    io << "  Alarms:\n";
    FLAG_OUT(state.alarms, OVERALL_ALARM);
    FLAG_OUT(state.alarms, ENGINE_TEMPERATURE_ALARM);
    FLAG_OUT(state.alarms, PM_VOLTAGE_ALARM);
    FLAG_OUT(state.alarms, OIL_PRESSURE_ALARM);
    FLAG_OUT(state.alarms, EXHAUST_TEMPERATURE_ALARM);
    FLAG_OUT(state.alarms, UAC1_ALARM);
    FLAG_OUT(state.alarms, IAC1_ALARM);
    FLAG_OUT(state.alarms, OIL_PRESSURE_HIGH_ALARM);
    FLAG_OUT(state.alarms, LOW_START_BATTERY_VOLTAGE_ALARM);
    FLAG_OUT(state.alarms, START_FAILURE);

    io << "  Start signals:\n";
    FLAG_OUT(state.start_signals, RUN_SIGNAL);
    FLAG_OUT(state.start_signals, START_BY_OPERATION_UNIT);

    io << "generator status: " << statusToString(state.generator_status) << "\n";

    return io;
}

static power_whisperpower::GensetState::Stage statusToGensetState(GeneratorStatus status)
{
    typedef power_whisperpower::GensetState::Stage Stage;

    switch (status) {
        case STATUS_NONE:
            return Stage::GENSET_STAGE_STOPPED;
        case STATUS_PRE_GLOW:
        case STATUS_GLOW:
        case STATUS_START:
        case STATUS_FIRST:
        case STATUS_ERROR_BYPASS:
        case STATUS_PRE_ERROR_BYPASS:
        case STATUS_PUMPING:
        case STATUS_RESTART:
        case STATUS_HYDROBOOST:
            return Stage::GENSET_STAGE_STARTING;
        case STATUS_SWITCH:
        case STATUS_STOP:
        case STATUS_FAILURE:
            return Stage::GENSET_STAGE_STOPPING;
        case STATUS_PRESENT:
            return Stage::GENSET_STAGE_RUNNING;
        case STATUS_UNKNOWN:
            return Stage::GENSET_STAGE_UNKNOWN;
    };
    return Stage::GENSET_STAGE_UNKNOWN;
}

power_whisperpower::GensetState GeneratorState::toGensetState() const
{
    power_whisperpower::GensetState common;
    common.time = time;
    common.failure_detected = (alarms != 0);
    common.stage = statusToGensetState(generator_status);
    return common;
}