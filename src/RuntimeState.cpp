#include <genset_whisperpower_ddc/RuntimeState.hpp>

using namespace genset_whisperpower_ddc;

using std::endl;

std::ostream& genset_whisperpower_ddc::operator << (std::ostream& io, RuntimeState const& state) {
    io << "Time: " << state.time << "\n";

    io << "total run time to be reset after maintenance: " << std::dec << state.total_runtime_hours << "h " << state.total_runtime_minutes << "min" << "\n";
    io << "historical run time: " << std::dec << state.historical_runtime_hours << "h " << state.historical_runtime_minutes << "min" << "\n";

    return io;
}