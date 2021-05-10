#ifndef GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP

#include <base/Time.hpp>
#include <genset_whisperpower_ddc/GeneratorStatus.hpp>

namespace genset_whisperpower_ddc {
    struct GeneratorState {
        base::Time time;

        float rotation_speed;
        int start_battery_voltage;

        enum Alarms {
            OVERALL_ALARM = 0x01,
            ENGINE_TEMPERATURE_ALARM = 0x02,
            PM_VOLTAGE_ALARM = 0x04,
            OIL_PRESSURE_ALARM = 0x08,
            EXHAUST_TEMPERATURE_ALARM = 0x10,
            UAC1_ALARM = 0x20,
            IAC1_ALARM = 0x40,
            OIL_PRESSURE_HIGH_ALARM = 0x80,
            LOW_START_BATTERY_VOLTAGE_ALARM = 0x0400,
            START_FAILURE = 0x1000,
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/Alarms */
        uint16_t alarms = 0;

        enum StartSignals {
            RUN_SIGNAL = 0x20,
            START_BY_OPERATION_UNIT = 0x80,
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/StartSignals */
        uint8_t start_signals = 0;

        GeneratorStatus generator_status;
    };

    std::ostream& operator << (std::ostream& io, GeneratorState const& state);
}

#endif