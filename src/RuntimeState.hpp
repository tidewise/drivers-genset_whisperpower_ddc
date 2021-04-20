#ifndef GENSET_WHISPERPOWER_DDC_RUNTIME_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_RUNTIME_STATE_HPP

#include <base/Time.hpp>

namespace genset_whisperpower_ddc {
    struct RuntimeState {
        base::Time time;
        
        int total_runtime_minutes; // Total run time to be reset after maintenance
        int total_runtime_hours;
        int historical_runtime_minutes;
        int historical_runtime_hours;
    };
}

#endif