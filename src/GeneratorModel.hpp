#ifndef GENSET_WHISPERPOWER_DDC_GENERATOR_MODEL_HPP
#define GENSET_WHISPERPOWER_DDC_GENERATOR_MODEL_HPP

#include <base/Time.hpp>

namespace genset_whisperpower_ddc {
    struct GeneratorModel {
        base::Time time;

        enum ModelDetection {
            MODEL_DETECTION_50HZ = 0x04,
            MODEL_DETECTION_60HZ = 0x08,
            MODEL_DETECTION_3_PHASE = 0x10,
            MODEL_DETECTION_MOBILE = 0x20
        };

        /** @meta bitfield /genset_whisperpower_ddc/GeneratorModel/ModelDetection */
        uint8_t model_detection = 0;

        int generator_type;
    };

    std::ostream& operator << (std::ostream& io, GeneratorModel const& model);
}

#endif