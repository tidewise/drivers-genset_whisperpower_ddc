#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <iodrivers_base/FixtureGTest.hpp>
#include <fcntl.h>
#include <thread>

using namespace std;
using namespace genset_whisperpower_ddc;
using testing::ElementsAreArray;
using base::Time;

struct VariableSpeedMasterTest : public ::testing::Test, iodrivers_base::Fixture<VariableSpeedMaster> {
    int pipeTX = -1;

    VariableSpeedMasterTest() {
    }

    ~VariableSpeedMasterTest() {
        if (pipeTX != -1) {
            close(pipeTX);
        }
    }

    void openPipe() {
        int pipes[2];
        ASSERT_EQ(pipe(pipes), 0);
        int rx = pipes[0];
        int tx = pipes[1];

        long fd_flags = fcntl(rx, F_GETFL);
        fcntl(rx, F_SETFL, fd_flags | O_NONBLOCK);

        driver.setFileDescriptor(rx, true);
        pipeTX = tx;
    }

    void writeToPipe(uint8_t const* bytes, int size) {
        ASSERT_EQ(write(pipeTX, bytes, size), 1);
    }
};

TEST_F(VariableSpeedMasterTest, it_throws_if_calling_readPacket) {
    driver.openURI("test://");
    // push one byte to get into extractPacket
    uint8_t buffer[1];
    pushDataToDriver(buffer, buffer + 1);
    ASSERT_THROW(driver.readPacket(buffer, 1024), std::logic_error);
}

TEST_F(VariableSpeedMasterTest, it_reads_a_frame) {
    openPipe();

    uint8_t bytes[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    thread writeThread([this,&bytes]{
        for (uint8_t i = 0; i < 16; ++i) {
            writeToPipe(bytes + i, 1);
            usleep(1000);
        }
    });

    Frame frame = driver.readFrame();
    ASSERT_EQ(variable_speed::TARGET_ADDRESS, frame.targetID);
    ASSERT_EQ(variable_speed::SOURCE_ADDRESS, frame.sourceID);
    ASSERT_EQ(0x02, frame.command);
    uint8_t payload[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    ASSERT_THAT(frame.payload, ElementsAreArray(payload));

    writeThread.join();
}

TEST_F(VariableSpeedMasterTest, it_writes_a_frame) {
    driver.openURI("test://");

    driver.writeFrame(0xF7, std::vector<uint8_t> {0, 1, 2, 3});
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0xF7, 0x00, 0x01, 0x02, 0x03, 0x06 };
    ASSERT_THAT(bytes, ElementsAreArray(expected));
}


TEST_F(VariableSpeedMasterTest, it_sends_command_F7) {
    driver.openURI("test://");
    
    driver.sendCommandF7(0x02);
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0xF7, 0x02, 0x00, 0x00, 0x00, 0x02 };
     ASSERT_THAT(bytes, ElementsAreArray(expected));
}

/*
TEST_F(VariableSpeedMasterTest, it_sends_command_02) {
    driver.openURI("test://");

    uint8_t statusA = variable_speed::getStatusByteA(true, false, true, false, true, false, true, false); // 0b01010101 = 0x55
    uint8_t statusB = variable_speed::getStatusByteB(false, true, false, true); // 0b10010000 = 0x90
    uint8_t statusC = variable_speed::getStatusByteC(true, false, true, false); // 0b00010100 = 0x14

    driver.sendCommand02(0x4A38, 0x00E6, statusA, statusB, statusC, 0x01, 0x04);
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0x38, 0x4A, 0xE6, 0x00, statusA, statusB, statusC, 0x01, 0x04, 0x00, 0x71 };
    ASSERT_THAT(bytes, ElementsAreArray(expected));
}

TEST_F(VariableSpeedMasterTest, it_sends_command_14) {
    driver.openURI("test://");

    driver.sendCommand14(0x1E, 0x000186A0, 0x0F, 0x00011170);
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x0E, 0x1E, 0xA0, 0x86, 0x01, 0x0F, 0x70, 0x11, 0x01, 0x00, 0x00, 0xED };
    ASSERT_THAT(bytes, ElementsAreArray(expected));
}
*/