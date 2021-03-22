rock_library(genset_whisperpower_ddc
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(genset_whisperpower_ddc_bin Main.cpp
    DEPS genset_whisperpower_ddc)

