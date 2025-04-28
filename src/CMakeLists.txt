rock_library(genset_whisperpower_ddc
    SOURCES VariableSpeed.cpp VariableSpeedMaster.cpp GeneratorState.cpp GeneratorModel.cpp
    HEADERS VariableSpeed.hpp VariableSpeedMaster.hpp Frame.hpp GeneratorStatus.hpp ControlCommand.hpp GeneratorState.hpp GeneratorModel.hpp RunTimeState.hpp
    DEPS_PKGCONFIG base-types iodrivers_base power_whisperpower)

rock_executable(genset_whisperpower_ddc_ctl Main.cpp
    DEPS genset_whisperpower_ddc)
