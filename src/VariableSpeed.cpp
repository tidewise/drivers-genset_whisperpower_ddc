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

std::vector<uint8_t> formatCommand02Data(uint16_t rpm, uint16_t udcStartBattery, uint8_t statusA,
                                     uint8_t statusB, uint8_t statusC, uint8_t generatorStatus,
                                     uint8_t generatorType) {
    std::vector<uint8_t> payload;
    payload.push_back(rpm & 0xFF);
    payload.push_back((rpm >> 8) & 0xFF);
    payload.push_back(udcStartBattery & 0xFF);
    payload.push_back((udcStartBattery >> 8) & 0xFF);
    payload.push_back(statusA);
    payload.push_back(statusB);
    payload.push_back(statusC);
    payload.push_back(generatorStatus);
    payload.push_back(generatorType);
    payload.push_back(0x00); // byte not used

    return payload;
}

std::vector<uint8_t> formatCommand14Data(uint8_t totalMinutes, uint32_t totalHours,
                                         uint8_t historicalMinutes, uint32_t historicalHours) {
    std::vector<uint8_t> payload;
    payload.push_back(totalMinutes);
    payload.push_back(totalHours & 0xFF);
    payload.push_back((totalHours >> 8) & 0xFF);
    payload.push_back((totalHours >> 16) & 0xFF);
    payload.push_back(historicalMinutes);
    payload.push_back(historicalHours & 0xFF);
    payload.push_back((historicalHours >> 8) & 0xFF);
    payload.push_back((historicalHours >> 16) & 0xFF);
    payload.push_back(0x00); // byte not used
    payload.push_back(0x00); // byte not used

    return payload;
}

uint8_t getStatusByteA(bool overallAlarm, bool engineTempAlarm, bool pmVoltageAlarm, bool oilPressAlarm,
                       bool exhaustTempAlarm, bool uac1Alarm, bool iac1Alarm, bool oilPressHighAlarm){
    return getStatusByte(overallAlarm, engineTempAlarm, pmVoltageAlarm, oilPressAlarm, 
                         exhaustTempAlarm, uac1Alarm, iac1Alarm, oilPressHighAlarm);
}

uint8_t getStatusByteB(bool lowStartBatteryVoltAlarm, bool startFailure, bool runSignal, bool startByOpUnit){
    return getStatusByte(false, false, lowStartBatteryVoltAlarm, false, startFailure, runSignal, false, startByOpUnit);
}

uint8_t getStatusByteC(bool mdlDetection50Hz, bool mdlDetection60Hz, bool mdlDetection3Phase, bool mobileMdlDetection){
    return getStatusByte(false, false, mdlDetection50Hz, mdlDetection60Hz, mdlDetection3Phase, mobileMdlDetection, false, false);
}

uint8_t getStatusByte(bool bit0, bool bit1, bool bit2, bool bit3, bool bit4, bool bit5, bool bit6, bool bit7){
    std::bitset<8> status;
    status.set(0, bit0);
    status.set(1, bit1);
    status.set(2, bit2);
    status.set(3, bit3);
    status.set(4, bit4);
    status.set(5, bit5);
    status.set(6, bit6);
    status.set(7, bit7);
    return (status.to_ulong & 0xFF);
}