rock_library(genset_whisperpower_ddc
    SOURCES VariableSpeed.cpp VariableSpeedMaster.cpp GeneratorState.cpp GeneratorModel.cpp RunTimeState.cpp ExportParameters.cpp
    HEADERS VariableSpeed.hpp VariableSpeedMaster.hpp Frame.hpp GeneratorStatus.hpp ControlCommand.hpp GeneratorState.hpp GeneratorModel.hpp RunTimeState.hpp ExportParameters.hpp
    DEPS_PKGCONFIG base-types iodrivers_base)

rock_executable(genset_whisperpower_ddc_ctl Main.cpp
    DEPS genset_whisperpower_ddc)
rock_executable(export_parameters ExportParameters.cpp
    DEPS genset_whisperpower_ddc
    DEPS_PKGCONFIG jsoncpp)
