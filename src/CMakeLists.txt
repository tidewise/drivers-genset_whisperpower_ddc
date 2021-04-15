rock_library(genset_whisperpower_ddc
    SOURCES VariableSpeed.cpp VariableSpeedMaster.cpp
    HEADERS VariableSpeed.hpp VariableSpeedMaster.hpp Frame.hpp GeneratorStatus.hpp ControlCommand.hpp
    DEPS_PKGCONFIG base-types iodrivers_base)

rock_executable(genset_whisperpower_ddc_ctl Main.cpp
    DEPS genset_whisperpower_ddc)
