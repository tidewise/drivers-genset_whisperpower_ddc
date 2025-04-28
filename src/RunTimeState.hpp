#ifndef GENSET_WHISPERPOWER_DDC_RUN_TIME_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_RUN_TIME_STATE_HPP

#include <base/Time.hpp>

namespace genset_whisperpower_ddc {
    struct RunTimeState {
        base::Time time;
        
        base::Time total_run_time; // Total run time to be reset after maintenance
        base::Time historical_run_time;
    };

    std::ostream& operator << (std::ostream& io, RunTimeState const& state);
}

#endif
