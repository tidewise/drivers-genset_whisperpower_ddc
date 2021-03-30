#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <iostream>

using namespace std;
using namespace genset_whisperpower_ddc;

int VariableSpeedMaster::extractPacket(uint8_t const* buffer, size_t bufferSize) const {
    throw std::logic_error("genset_whisperpower_dcc::VariableSpeedMaster should be read only using readRaw");
}

VariableSpeedMaster::VariableSpeedMaster() : iodrivers_base::Driver(variable_speed::FRAME_MAX_SIZE * 10) {
    setReadTimeout(base::Time::fromSeconds(1));
    m_read_buffer.resize(MAX_PACKET_SIZE);
    m_write_buffer.resize(MAX_PACKET_SIZE);
    m_frame.payload.reserve(variable_speed::FRAME_MAX_SIZE);
}


Frame VariableSpeedMaster::readFrame() {
    Frame result;
    readFrame(result);
    return result;
}

void VariableSpeedMaster::readFrame(Frame& frame) {
    int c = readRaw(&m_read_buffer[0], m_read_buffer.size(),
                    getReadTimeout(), getReadTimeout(), m_interframe_delay);

    variable_speed::parseFrame(frame, &m_read_buffer[0], &m_read_buffer[c]);
}

void VariableSpeedMaster::writeFrame(uint8_t command, std::vector<uint8_t> const& payload) {
    uint8_t* start = &m_write_buffer[0];
    uint8_t const* end = variable_speed::formatFrame(start, variable_speed::TARGET_ADDRESS, variable_speed::SOURCE_ADDRESS, command, payload);
    writePacket(&m_write_buffer[0], end - start);
}