rock_library(genset_whisperpower_ddc
    SOURCES FixedRPM.cpp FixedRPMMaster.cpp
    HEADERS FixedRPM.hpp FixedRPMMaster.hpp Frame.hpp
    DEPS_PKGCONFIG base-types iodrivers_base)

rock_executable(genset_whisperpower_ddc_bin Main.cpp
    DEPS genset_whisperpower_ddc)
