#ifndef GENSET_WHISPERPOWER_DDC_GENERATOR_STATUS_HPP
#define GENSET_WHISPERPOWER_DDC_GENERATOR_STATUS_HPP

namespace genset_whisperpower_ddc {
    enum GeneratorStatus {
                STATUS_NONE = 0x00,
                STATUS_PRE_GLOW = 0x01,
                STATUS_GLOW = 0x02,
                STATUS_START = 0x03,
                STATUS_SWITCH = 0x04,
                STATUS_STOP = 0x05,
                STATUS_FAILURE = 0x06,
                STATUS_PRESENT = 0x07,
                STATUS_FIRST = 0x08,
                STATUS_ERROR_BYPASS = 0x09,
                STATUS_PRE_ERROR_BYPASS = 0x0A,
                STATUS_PUMPING = 0x0B,
                STATUS_RESTART = 0X0C,
                STATUS_HYDROBOOST = 0x0D,
                STATUS_UNKNOWN = 0X0E
    };
}

#endif