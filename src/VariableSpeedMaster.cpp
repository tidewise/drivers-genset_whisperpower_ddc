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

void VariableSpeedMaster::Command02(uint16_t rpm, uint16_t udcStartBattery, uint8_t statusA,
                                    uint8_t statusB, uint8_t statusC, uint8_t generatorStatus,
                                    uint8_t generatorType) {
    uint8_t* buffer_start = &m_write_buffer[0];
    std::vector<uint8_t> payload = variable_speed::formatCommand02Data(rpm, udcStartBattery, statusA, statusB, statusC, generatorStatus, generatorType);
    uint8_t const* buffer_end = variable_speed::formatFrame(
        buffer_start, variable_speed::TARGET_ADDRESS, variable_speed::SOURCE_ADDRESS, 0x02, payload
    );
    writePacket(buffer_start, buffer_end - buffer_start);
}

void VariableSpeedMaster::Command14(uint8_t totalMinutes, uint32_t totalHours,
                                    uint8_t historicalMinutes, uint32_t historicalHours) {
    uint8_t* buffer_start = &m_write_buffer[0];
    std::vector<uint8_t> payload = variable_speed::formatCommand14Data(totalMinutes, totalHours, historicalMinutes, historicalHours);
    uint8_t const* buffer_end = variable_speed::formatFrame(
        buffer_start, variable_speed::TARGET_ADDRESS, variable_speed::SOURCE_ADDRESS, 0x0E, payload
    );
    writePacket(buffer_start, buffer_end - buffer_start);
}