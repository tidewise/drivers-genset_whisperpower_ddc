#ifndef EXPORT_PARAMETERS_HPP
#define EXPORT_PARAMETERS_HPP

namespace genset_whisperpower_ddc {
    class ExportParameters {
    public:
        ExportParameters();
        ~ExportParameters();

        void exportParameters(pair<GeneratorState, GeneratorModel> genset,
            RunTimeState run_time_state);
    };
}

#endif
