#ifndef GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_GENERATOR_STATE_HPP

#include <base/Time.hpp>
#include <genset_whisperpower_ddc/GeneratorStatus.hpp>

namespace genset_whisperpower_ddc {
    struct GeneratorState {
        base::Time time;

        int rpm;
        int udc_start_battery;
        enum StatusA {
            OVERALL_ALARM = 0x01,
            ENGINE_TEMPERATURE_ALARM = 0x02,
            PM_VOLTAGE_ALARM = 0x04,
            OIL_PRESSURE_ALARM = 0x08,
            EXHAUST_TEMPERATURE_ALARM = 0x10,
            UAC1_ALARM = 0x20,
            IAC1_ALARM = 0x40,
            OIL_PRESSURE_HIGH_ALARM = 0x80
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/StatusA */
        uint8_t statusA = 0;

        enum StatusB {
            LOW_START_BATTERY_VOLTAGE_ALARM = 0x04,
            START_FAILURE = 0x10,
            RUN_SIGNAL = 0x20,
            START_BY_OPERATION_UNIT = 0x80
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/StatusB */
        uint8_t statusB = 0;

        enum StatusC {
            MODEL_DETECTION_50HZ = 0x04,
            MODEL_DETECTION_60HZ = 0x08,
            MODEL_DETECTION_3_PHASE = 0x10,
            MODEL_DETECTION_MOBILE = 0x20
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorState/StatusC */
        uint8_t statusC = 0;

        GeneratorStatus generator_status;
        int generator_type;
    };
}

#endif