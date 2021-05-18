#include <genset_whisperpower_ddc/GeneratorModel.hpp>

using namespace genset_whisperpower_ddc;

using std::endl;

#define FLAG_OUT(field, flag) \
    if (field & GeneratorModel::flag) { \
        io << "    " #flag "\n"; \
    }

std::ostream& genset_whisperpower_ddc::operator << (std::ostream& io, GeneratorModel const& model) {
    io << "Time: " << model.time << "\n";

    io << "  Model detection:\n";
    FLAG_OUT(model.model_detection, MODEL_DETECTION_50HZ);
    FLAG_OUT(model.model_detection, MODEL_DETECTION_60HZ);
    FLAG_OUT(model.model_detection, MODEL_DETECTION_3_PHASE);
    FLAG_OUT(model.model_detection, MODEL_DETECTION_MOBILE);

    io << "generator type: " << std::dec << model.generator_type;

    return io;
}