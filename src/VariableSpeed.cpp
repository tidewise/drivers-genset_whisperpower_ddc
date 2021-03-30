#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <cstring>

using namespace std;
using namespace genset_whisperpower_ddc;

uint8_t* variable_speed::formatFrame(uint8_t* buffer, uint16_t targetID, uint16_t sourceID, uint8_t command,
                     std::vector<uint8_t> const& payload) {

    auto begin = &payload[0];
    return variable_speed::formatFrame(buffer, targetID, sourceID, command, begin, begin + payload.size());
}
uint8_t* variable_speed::formatFrame(uint8_t* buffer, uint16_t targetID, uint16_t sourceID, uint8_t command,
                                uint8_t const* payloadStart, uint8_t const* payloadEnd) {

    int payloadSize = payloadEnd - payloadStart;
    if (payloadSize + FRAME_OVERHEAD_SIZE != SENT_FRAME_SIZE) {
        throw std::invalid_argument("variable_speed::formatFrame: frame size would be different "\
                                    "from specified size of 16");
    }

    buffer[0] = targetID & 0xFF;
    buffer[1] = (targetID >> 8) & 0xFF;
    buffer[2] = sourceID & 0xFF;
    buffer[3] = (sourceID >> 8) & 0xFF;
    buffer[4] = command;
    auto bufferPayload = buffer + FRAME_HEADER_SIZE;
    memcpy(bufferPayload, payloadStart, payloadSize);
    uint8_t bufferChecksum = checksum(buffer, bufferPayload + payloadSize);
    *(bufferPayload + payloadSize) = bufferChecksum;
    return bufferPayload + payloadSize + 1;
}

static void validateBufferSize(uint8_t const* start, uint8_t const* end,
                               char const* context) {
    if (end - start != variable_speed::RECEIVED_FRAME_SIZE) {
        throw variable_speed::WrongSize(
            string(context) + ": "
            "expected " + to_string(variable_speed::RECEIVED_FRAME_SIZE) + " bytes, "
            "but got " + to_string(end - start)
        );
    }
}

Frame variable_speed::parseFrame(uint8_t const* start, uint8_t const* end) {
    Frame result;
    parseFrame(result, start, end);
    return result;
}

void variable_speed::parseFrame(Frame& frame, uint8_t const* start, uint8_t const* end) {
    validateBufferSize(start, end, "variable_speed::parseFrame");
    if (!isChecksumValid(start, end)) {
        throw InvalidChecksum("variable_speed::parseFrame: checksum failed");
    }

    uint16_t lsbTargetID = start[0];
    uint16_t msbTargetID = start[1];
    frame.targetID = (msbTargetID << 8) | lsbTargetID;
    uint16_t lsbSourceID = start[2];
    uint16_t msbSourceID = start[3];
    frame.sourceID = (msbSourceID << 8) | lsbSourceID;
    frame.command = start[4];
    frame.payload.resize((end - start) - FRAME_OVERHEAD_SIZE);
    std::copy(start + FRAME_HEADER_SIZE, end - 1, frame.payload.begin());
}


bool variable_speed::isChecksumValid(uint8_t const* start, uint8_t const* end) {
    validateBufferSize(start, end, "variable_speed::isChecksumValid");
    uint8_t expected = checksum(start, end - 1);
    return (end[-1] == expected);
}

uint8_t variable_speed::checksum(uint8_t const* start, uint8_t const* end) {
    uint16_t checksum = 0;
    for (uint8_t const* it = start; it != end; ++it) {
        checksum += (uint16_t)*it;
    }
    return (uint8_t) (checksum & 0xFF); // get lowest byte
}