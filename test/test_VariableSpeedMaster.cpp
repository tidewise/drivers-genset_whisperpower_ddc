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


TEST_F(VariableSpeedMasterTest, it_sends_control_command) {
    driver.openURI("test://");

    driver.sendControlCommand(0x02);
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0xF7, 0x02, 0x00, 0x00, 0x00, 0x02 };
     ASSERT_THAT(bytes, ElementsAreArray(expected));
}

TEST_F(VariableSpeedMasterTest, it_parses_generator_state) {
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

    ASSERT_EQ(0x02, frame.command);

    Time now = Time::now();
    GeneratorState generatorState = driver.parseGeneratorState(frame.payload, now);

    GeneratorState expectedState;
    expectedState.time = now;
    expectedState.rpm = 0x0100;
    expectedState.udc_start_battery = 0x0302;
    expectedState.statusA = 0x04;
    expectedState.statusB = 0x05;
    expectedState.statusC = 0x06;
    expectedState.generator_status = GeneratorStatus::STATUS_PRESENT;
    expectedState.generator_type = 0x08;

    ASSERT_EQ(generatorState.time, expectedState.time);
    ASSERT_EQ(generatorState.rpm, expectedState.rpm);
    ASSERT_EQ(generatorState.udc_start_battery, expectedState.udc_start_battery);
    ASSERT_EQ(generatorState.statusA, expectedState.statusA);
    ASSERT_EQ(generatorState.statusB, expectedState.statusB);
    ASSERT_EQ(generatorState.statusC, expectedState.statusC);
    ASSERT_EQ(generatorState.generator_status, expectedState.generator_status);
    ASSERT_EQ(generatorState.generator_type, expectedState.generator_type);

    writeThread.join();
}

TEST_F(VariableSpeedMasterTest, it_parses_runtime_state) {
    openPipe();

    uint8_t bytes[] = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x0E, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x44 };
    thread writeThread([this,&bytes]{
        for (uint8_t i = 0; i < 16; ++i) {
            writeToPipe(bytes + i, 1);
            usleep(1000);
        }
    });

    Frame frame = driver.readFrame();

    ASSERT_EQ(0x0E, frame.command);

    Time now = Time::now();
    RuntimeState runtimeState = driver.parseRuntimeState(frame.payload, now);

    RuntimeState expectedState;
    expectedState.time = now;
    expectedState.total_runtime_minutes = 0x00;
    expectedState.total_runtime_hours = 0x030201;
    expectedState.historical_runtime_minutes = 0x04;
    expectedState.historical_runtime_hours = 0x070605;

    ASSERT_EQ(runtimeState.time, expectedState.time);
    ASSERT_EQ(runtimeState.total_runtime_minutes, expectedState.total_runtime_minutes);
    ASSERT_EQ(runtimeState.total_runtime_hours, expectedState.total_runtime_hours);
    ASSERT_EQ(runtimeState.historical_runtime_minutes, expectedState.historical_runtime_minutes);
    ASSERT_EQ(runtimeState.historical_runtime_hours, expectedState.historical_runtime_hours);

    writeThread.join();
}
