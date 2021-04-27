#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <iostream>

using namespace std;
using namespace genset_whisperpower_ddc;

int VariableSpeedMaster::extractPacket(uint8_t const* buffer, size_t bufferSize) const {
    throw std::logic_error("genset_whisperpower_dcc::VariableSpeedMaster should be read only using readRaw");
}

VariableSpeedMaster::VariableSpeedMaster() : iodrivers_base::Driver(variable_speed::RECEIVED_FRAME_SIZE * 10) {
    setReadTimeout(base::Time::fromSeconds(1));
    m_read_buffer.resize(MAX_PACKET_SIZE);
    m_write_buffer.resize(MAX_PACKET_SIZE);
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

void VariableSpeedMaster::sendControlCommand(uint8_t controlCommand) {
    std::vector<uint8_t> payload = variable_speed::formatCommandF7Data(controlCommand);
    writeFrame(0xF7, payload);
}

GeneratorState VariableSpeedMaster::parseGeneratorState(std::vector<uint8_t> payload, base::Time const& time)
{
    GeneratorState generator_state;
    generator_state.time = time;
    generator_state.rpm = (payload[1] << 8) | payload[0];
    generator_state.udc_start_battery = (payload[3] << 8) | payload[2];
    generator_state.status = (payload[6] << 16) | (payload[5] << 8) | payload[4];
    if (payload[7] < 0x0E){
        generator_state.generator_status = static_cast<GeneratorStatus>(payload[7]);
    }
    else{
        generator_state.generator_status = STATUS_UNKNOWN;
    }
    generator_state.generator_type = payload[8];

    return generator_state;
}

RuntimeState VariableSpeedMaster::parseRuntimeState(std::vector<uint8_t> payload, base::Time const& time)
{
    RuntimeState runtime_state;
    runtime_state.time = time;
    int minutes = payload[0];
    int hours = (payload[3] << 16) | (payload[2] << 8) | payload[1];
    runtime_state.total_runtime = base::Time::fromSeconds((hours * 60 * 60) + (minutes * 60));
    minutes = payload[4];
    hours = (payload[7] << 16) | (payload[6] << 8) | payload[5];
    runtime_state.historical_runtime = base::Time::fromSeconds((hours * 60 * 60) + (minutes * 60));
    return runtime_state;
}