#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <genset_whisperpower_ddc/FixedRPM.hpp>

using namespace std;
using namespace genset_whisperpower_ddc;
using testing::ElementsAreArray;

struct FixedRPMTest : public ::testing::Test {
};

TEST_F(FixedRPMTest, it_computes_the_checksum) {
    uint8_t bytes[] = { 101, 102, 103, 104, 105, 106 , 107, 108, 109, 110, 111, 112, 113, 114, 115};
    uint8_t checksum = fixed_RPM::checksum(bytes, bytes + 15);
    /** 101+102+103+104+105+106+107+108+109+110+111+112+113+114+115 = 1620
     *  1620 = 11001010100b
     *  lowest byte: 01010100b = 0x54
     */
    ASSERT_EQ(0x54, checksum);
}

TEST_F(FixedRPMTest, it_formats_a_frame) {
    uint8_t buffer[16];
    uint8_t payload[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t* bufferEnd = fixed_RPM::formatFrame(buffer, fixed_RPM::TARGET_ADDRESS, fixed_RPM::SOURCE_ADDRESS, 0x1, payload, payload + 10);
    ASSERT_EQ(16, bufferEnd - buffer);

    /** checksum: 
     * fixed_RPM::TARGET_ADDRESS = 0x0081
     * fixed_RPM::SOURCE_ADDRESS = 0x0080
     * command: 0x1
     * 0+1+2+3+4+5+6+7+8+9 = 45 = 101101b = 0x2D
     * checksum = LSB(0x81 + 0x00 + 0x80 + 0x00 + 0x1 + 0x2D) = LSB(0x12F) = 0x2F
     */
    uint8_t expected[] = { fixed_RPM::TARGET_ADDRESS & 0xFF, (fixed_RPM::TARGET_ADDRESS >> 8) & 0xFF, fixed_RPM::SOURCE_ADDRESS & 0xFF,
                           (fixed_RPM::SOURCE_ADDRESS >> 8) & 0xFF, 0x1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x2F };
    ASSERT_THAT(std::vector<uint8_t>(buffer, bufferEnd),
                ElementsAreArray(expected));
}

TEST_F(FixedRPMTest, it_throws_if_the_resulting_frame_size_would_be_different_from_16_bytes) {
    uint8_t buffer[0];
    ASSERT_THROW(fixed_RPM::formatFrame(nullptr, fixed_RPM::TARGET_ADDRESS,
                 fixed_RPM::SOURCE_ADDRESS, 0x1, buffer, buffer + 11),
                 std::invalid_argument);
}

TEST_F(FixedRPMTest, it_parses_a_frame) {
    uint8_t bytes[] = { fixed_RPM::TARGET_ADDRESS & 0xFF, (fixed_RPM::TARGET_ADDRESS >> 8) & 0xFF, fixed_RPM::SOURCE_ADDRESS & 0xFF,
                           (fixed_RPM::SOURCE_ADDRESS >> 8) & 0xFF, 0x1, 0, 1, 2, 3, 0x2F };
    Frame frame = fixed_RPM::parseFrame(bytes, bytes + 10);
    ASSERT_EQ(fixed_RPM::TARGET_ADDRESS, frame.targetID);
    ASSERT_EQ(fixed_RPM::SOURCE_ADDRESS, frame.sourceID);
    ASSERT_EQ(0x1, frame.command);

    uint8_t payload[4] = { 0, 1, 2, 3 };
    ASSERT_THAT(frame.payload, ElementsAreArray(payload));
}

TEST_F(FixedRPMTest, it_throws_if_attempting_to_parse_a_buffer_of_wrong_size) {
    uint8_t bytes[0];
    ASSERT_THROW(fixed_RPM::parseFrame(bytes, bytes + 16), fixed_RPM::WrongSize);
}

TEST_F(FixedRPMTest, it_throws_if_the_checksum_check_fails) {
    uint8_t bytes[] = { fixed_RPM::TARGET_ADDRESS & 0xFF, (fixed_RPM::TARGET_ADDRESS >> 8) & 0xFF, fixed_RPM::SOURCE_ADDRESS & 0xFF,
                           (fixed_RPM::SOURCE_ADDRESS >> 8) & 0xFF, 0x1, 0, 1, 2, 3, 0x30 };
    ASSERT_THROW(fixed_RPM::parseFrame(bytes, bytes + 10), fixed_RPM::InvalidChecksum);
}

TEST_F(FixedRPMTest, it_throws_if_the_buffer_is_of_wrong_size_when_validating_checksum) {
    uint8_t bytes[0];
    ASSERT_THROW(fixed_RPM::isChecksumValid(bytes, bytes + 16), fixed_RPM::WrongSize);
}

TEST_F(FixedRPMTest, it_recognizes_a_valid_checksum) {
    uint8_t frame[] = { fixed_RPM::TARGET_ADDRESS & 0xFF, (fixed_RPM::TARGET_ADDRESS >> 8) & 0xFF, fixed_RPM::SOURCE_ADDRESS & 0xFF,
                           (fixed_RPM::SOURCE_ADDRESS >> 8) & 0xFF, 0x1, 0, 1, 2, 3, 0x2F };
    ASSERT_TRUE(fixed_RPM::isChecksumValid(frame, frame + 10));
}
