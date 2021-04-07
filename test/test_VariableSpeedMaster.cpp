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
