#ifndef GENSET_WHISPERPOWER_DDC_RUNTIME_STATE_HPP
#define GENSET_WHISPERPOWER_DDC_RUNTIME_STATE_HPP

#include <base/Time.hpp>

namespace genset_whisperpower_ddc {
    struct RuntimeState {
        base::Time time;
        
        base::Time total_runtime; // Total run time to be reset after maintenance
        base::Time historical_runtime;
    };

    std::ostream& operator << (std::ostream& io, RuntimeState const& state);
}

#endif