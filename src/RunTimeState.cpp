#include <genset_whisperpower_ddc/RunTimeState.hpp>

using namespace genset_whisperpower_ddc;

using std::endl;

std::ostream& genset_whisperpower_ddc::operator << (std::ostream& io, RunTimeState const& state) {
    io << "Time: " << state.time << "\n";

    io << "total run time to be reset after maintenance: " << std::dec << state.total_run_time << "\n";
    io << "historical run time: " << std::dec << state.historical_run_time << "\n";

    return io;
}