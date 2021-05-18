#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>

using namespace std;
using namespace genset_whisperpower_ddc;
using testing::ElementsAreArray;

struct VariableSpeedTest : public ::testing::Test {
};

TEST_F(VariableSpeedTest, it_computes_the_checksum) {
    uint8_t bytes[] = { 101, 102, 103, 104, 105, 106 , 107, 108, 109, 110, 111, 112, 113, 114, 115};
    uint8_t checksum = variable_speed::checksum(bytes, bytes + 15);
    /** 101+102+103+104+105+106+107+108+109+110+111+112+113+114+115 = 1620
     *  1620 = 0b11001010100
     *  lowest byte: 0b01010100 = 0x54
     */
    ASSERT_EQ(0x54, checksum);
}

TEST_F(VariableSpeedTest, it_formats_a_frame) {
    uint8_t buffer[10];
    uint8_t payload[4] = { 0, 1, 2, 3 };
    uint8_t* bufferEnd = variable_speed::formatFrame(buffer, variable_speed::TARGET_ADDRESS, variable_speed::SOURCE_ADDRESS, variable_speed::PACKET_START_STOP, payload, payload + 4);
    ASSERT_EQ(10, bufferEnd - buffer);

    /** checksum: 
     * variable_speed::TARGET_ADDRESS = 0x0081
     * variable_speed::SOURCE_ADDRESS = 0x0088
     * command: 0xF7
     * checksum = LSB(0x81 + 0x00 + 0x88 + 0x00 + 0x0F7 + 0X00 + 0x01 + 0x02 + 0x03) = LSB(0x206) = 0x06
     */
    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_START_STOP, 0, 1, 2, 3, 0x06 };
    ASSERT_THAT(std::vector<uint8_t>(buffer, bufferEnd),
                ElementsAreArray(expected));
}

TEST_F(VariableSpeedTest, it_throws_if_the_resulting_frame_size_would_be_different_from_10_bytes) {
    uint8_t buffer[0];
    ASSERT_THROW(variable_speed::formatFrame(nullptr, variable_speed::TARGET_ADDRESS,
                 variable_speed::SOURCE_ADDRESS, variable_speed::PACKET_START_STOP, buffer, buffer + 11),
                 std::invalid_argument);
}

TEST_F(VariableSpeedTest, it_parses_a_frame) {
    uint8_t bytes[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    Frame frame = variable_speed::parseFrame(bytes, bytes + 16);
    ASSERT_EQ(variable_speed::TARGET_ADDRESS, frame.targetID);
    ASSERT_EQ(variable_speed::SOURCE_ADDRESS, frame.sourceID);
    ASSERT_EQ(0x02, frame.command);

    uint8_t payload[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    ASSERT_THAT(frame.payload, ElementsAreArray(payload));
}

TEST_F(VariableSpeedTest, it_throws_if_attempting_to_parse_a_buffer_of_wrong_size) {
    uint8_t bytes[0];
    ASSERT_THROW(variable_speed::parseFrame(bytes, bytes + 10), variable_speed::WrongSize);
}

TEST_F(VariableSpeedTest, it_throws_if_the_checksum_check_fails) {
    uint8_t bytes[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x39 };
    ASSERT_THROW(variable_speed::parseFrame(bytes, bytes + 16), variable_speed::InvalidChecksum);
}

TEST_F(VariableSpeedTest, it_throws_if_the_buffer_is_of_wrong_size_when_validating_checksum) {
    uint8_t bytes[0];
    ASSERT_THROW(variable_speed::isChecksumValid(bytes, bytes + 10), variable_speed::WrongSize);
}

TEST_F(VariableSpeedTest, it_recognizes_a_valid_checksum) {
    uint8_t frame[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    ASSERT_TRUE(variable_speed::isChecksumValid(frame, frame + 16));
}
