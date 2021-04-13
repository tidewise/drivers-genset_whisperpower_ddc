#ifndef GENSET_WHISPERPOWER_DDC_CONTROL_COMMAND_HPP
#define GENSET_WHISPERPOWER_DDC_CONTROL_COMMAND_HPP

namespace genset_whisperpower_ddc {
    enum ControlCommand {
        start = 1,
        stop = 2,
        keepAlive = 3
    };
}

#endif