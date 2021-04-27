#ifndef GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP

#include <base/Time.hpp>
#include <genset_whisperpower_ddc/GeneratorStatus.hpp>

namespace genset_whisperpower_ddc {
    struct GeneratorState {
        base::Time time;

        int rpm;
        int udc_start_battery;
        enum Status {
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
            RUN_SIGNAL = 0x2000,
            START_BY_OPERATION_UNIT = 0x8000,
            MODEL_DETECTION_50HZ = 0x040000,
            MODEL_DETECTION_60HZ = 0x080000,
            MODEL_DETECTION_3_PHASE = 0x100000,
            MODEL_DETECTION_MOBILE = 0x200000
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/Status */
        uint32_t status = 0;

        GeneratorStatus generator_status;
        int generator_type;
    };

    std::ostream& operator << (std::ostream& io, GeneratorState const& state);
}

#endif