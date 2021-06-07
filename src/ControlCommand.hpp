#ifndef GENSET_WHISPERPOWER_DDC_CONTROL_COMMAND_HPP
#define GENSET_WHISPERPOWER_DDC_CONTROL_COMMAND_HPP

namespace genset_whisperpower_ddc {
    enum ControlCommand {
        CONTROL_CMD_START = 1,
        CONTROL_CMD_STOP = 2,
        CONTROL_CMD_KEEP_ALIVE = 3
    };
}

#endif