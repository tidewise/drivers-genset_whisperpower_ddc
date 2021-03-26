#ifndef _GENSET_WHISPERPOWER_DDC_FRAME_HPP_
#define _GENSET_WHISPERPOWER_DDC_FRAME_HPP_

#include <vector>
#include <cstdint>

namespace genset_whisperpower_ddc {
    /**
     * A genset_whisperpower_ddc frame
     */
    struct Frame {
        uint16_t targetID;
        uint16_t sourceID;
        uint8_t command;
        std::vector<uint8_t> payload;
    };
}

#endif