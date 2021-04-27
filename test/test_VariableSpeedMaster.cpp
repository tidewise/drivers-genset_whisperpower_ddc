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
    VariableSpeedMasterTest() {
    }

    ~VariableSpeedMasterTest() {
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
    driver.openURI("test://");

    std::vector<uint8_t> buffer = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();
    ASSERT_EQ(variable_speed::TARGET_ADDRESS, frame.targetID);
    ASSERT_EQ(variable_speed::SOURCE_ADDRESS, frame.sourceID);
    ASSERT_EQ(0x02, frame.command);
    uint8_t payload[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    ASSERT_THAT(frame.payload, ElementsAreArray(payload));
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
    driver.openURI("test://");

    std::vector<uint8_t> buffer = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x02, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();

    ASSERT_EQ(0x02, frame.command);

    Time now = Time::now();
    GeneratorState generatorState = driver.parseGeneratorState(frame.payload, now);

    GeneratorState expectedState;
    expectedState.time = now;
    expectedState.rpm = 0x0100;
    expectedState.udc_start_battery = 0x0302;
    expectedState.status = 0x060504;
    expectedState.generator_status = GeneratorStatus::STATUS_PRESENT;
    expectedState.generator_type = 0x08;

    ASSERT_EQ(generatorState.time, expectedState.time);
    ASSERT_EQ(generatorState.rpm, expectedState.rpm);
    ASSERT_EQ(generatorState.udc_start_battery, expectedState.udc_start_battery);
    ASSERT_EQ(generatorState.status, expectedState.status);
    ASSERT_EQ(generatorState.generator_status, expectedState.generator_status);
    ASSERT_EQ(generatorState.generator_type, expectedState.generator_type);
}

TEST_F(VariableSpeedMasterTest, it_parses_runtime_state) {
    driver.openURI("test://");

    std::vector<uint8_t> buffer = { variable_speed::TARGET_ADDRESS & 0xFF, (variable_speed::TARGET_ADDRESS >> 8) & 0xFF, variable_speed::SOURCE_ADDRESS & 0xFF,
                           (variable_speed::SOURCE_ADDRESS >> 8) & 0xFF, 0x0E, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x44 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();

    ASSERT_EQ(0x0E, frame.command);

    Time now = Time::now();
    RuntimeState runtimeState = driver.parseRuntimeState(frame.payload, now);

    RuntimeState expectedState;
    expectedState.time = now;
    expectedState.total_runtime = Time::fromSeconds((0x030201 * 60 * 60) + (0x00 * 60));
    expectedState.historical_runtime =  Time::fromSeconds((0x070605 * 60 * 60) + (0x04 * 60));

    ASSERT_EQ(runtimeState.time, expectedState.time);
    ASSERT_EQ(runtimeState.total_runtime, expectedState.total_runtime);
    ASSERT_EQ(runtimeState.historical_runtime, expectedState.historical_runtime);
    ASSERT_EQ(runtimeState.historical_runtime, expectedState.historical_runtime);
}
