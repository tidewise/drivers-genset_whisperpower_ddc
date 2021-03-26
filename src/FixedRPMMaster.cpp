#include <genset_whisperpower_ddc/FixedRPMMaster.hpp>
#include <genset_whisperpower_ddc/FixedRPM.hpp>
#include <iostream>

using namespace std;
using namespace genset_whisperpower_ddc;

int FixedRPMMaster::extractPacket(uint8_t const* buffer, size_t bufferSize) const {
    throw std::logic_error("genset_whisperpower_dcc::FixedRPMMaster should be read only using readRaw");
}

FixedRPMMaster::FixedRPMMaster() : iodrivers_base::Driver(fixed_RPM::FRAME_MAX_SIZE * 10) {
    setReadTimeout(base::Time::fromSeconds(1));
    m_read_buffer.resize(MAX_PACKET_SIZE);
    m_write_buffer.resize(MAX_PACKET_SIZE);
    m_frame.payload.reserve(fixed_RPM::FRAME_MAX_SIZE);
}


Frame FixedRPMMaster::readFrame() {
    Frame result;
    readFrame(result);
    return result;
}

void FixedRPMMaster::readFrame(Frame& frame) {
    int c = readRaw(&m_read_buffer[0], m_read_buffer.size(),
                    getReadTimeout(), getReadTimeout(), m_interframe_delay);

    fixed_RPM::parseFrame(frame, &m_read_buffer[0], &m_read_buffer[c]);
}

void FixedRPMMaster::writeFrame(uint8_t command, std::vector<uint8_t> const& payload) {
    uint8_t* start = &m_write_buffer[0];
    uint8_t const* end = fixed_RPM::formatFrame(start, fixed_RPM::TARGET_ADDRESS, fixed_RPM::SOURCE_ADDRESS, command, payload);
    writePacket(&m_write_buffer[0], end - start);
}