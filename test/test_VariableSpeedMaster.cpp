#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <iodrivers_base/FixtureGTest.hpp>
#include <fcntl.h>
#include <thread>

#define PI 3.141592653589793

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

    std::vector<uint8_t> buffer = { variable_speed::PANELS_ADDRESS & 0xFF, (variable_speed::PANELS_ADDRESS >> 8) & 0xFF, variable_speed::DDC_CONTROLLER_ADDRESS & 0xFF,
                           (variable_speed::DDC_CONTROLLER_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_GENERATOR_STATE_AND_MODEL, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();
    ASSERT_EQ(variable_speed::PANELS_ADDRESS, frame.targetID);
    ASSERT_EQ(variable_speed::DDC_CONTROLLER_ADDRESS, frame.sourceID);
    ASSERT_EQ(variable_speed::PACKET_GENERATOR_STATE_AND_MODEL, frame.command);
    uint8_t payload[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    ASSERT_THAT(frame.payload, ElementsAreArray(payload));
}

TEST_F(VariableSpeedMasterTest, it_writes_a_frame) {
    driver.openURI("test://");

    driver.writeFrame(variable_speed::PACKET_START_STOP, std::vector<uint8_t> {0, 1, 2, 3});
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::DDC_CONTROLLER_ADDRESS & 0xFF, (variable_speed::DDC_CONTROLLER_ADDRESS >> 8) & 0xFF, variable_speed::PANELS_ADDRESS & 0xFF,
                           (variable_speed::PANELS_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_START_STOP, 0x00, 0x01, 0x02, 0x03, 0x06 };
    ASSERT_THAT(bytes, ElementsAreArray(expected));
}


TEST_F(VariableSpeedMasterTest, it_sends_control_command) {
    driver.openURI("test://");

    driver.sendControlCommand(variable_speed::PACKET_GENERATOR_STATE_AND_MODEL);
    auto bytes = readDataFromDriver();

    uint8_t expected[] = { variable_speed::DDC_CONTROLLER_ADDRESS & 0xFF, (variable_speed::DDC_CONTROLLER_ADDRESS >> 8) & 0xFF, variable_speed::PANELS_ADDRESS & 0xFF,
                           (variable_speed::PANELS_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_START_STOP, variable_speed::PACKET_GENERATOR_STATE_AND_MODEL, 0x00, 0x00, 0x00, 0x02 };
     ASSERT_THAT(bytes, ElementsAreArray(expected));
}

TEST_F(VariableSpeedMasterTest, it_parses_generator_state_and_model) {
    driver.openURI("test://");

    std::vector<uint8_t> buffer = { variable_speed::PANELS_ADDRESS & 0xFF, (variable_speed::PANELS_ADDRESS >> 8) & 0xFF, variable_speed::DDC_CONTROLLER_ADDRESS & 0xFF,
                           (variable_speed::DDC_CONTROLLER_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_GENERATOR_STATE_AND_MODEL, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x38 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();

    ASSERT_EQ(variable_speed::PACKET_GENERATOR_STATE_AND_MODEL, frame.command);

    Time now = Time::now();
    std::pair<GeneratorState, GeneratorModel> generatorStateAndModel = driver.parseGeneratorStateAndModel(frame.payload, now);

    GeneratorState expectedState;
    expectedState.time = now;
    expectedState.rotation_speed = ((2*PI)/60) * 0x0100;
    expectedState.start_battery_voltage = 0.01 * 0x0302;
    expectedState.alarms = 0x0504;
    expectedState.start_signals = 0x05;
    expectedState.generator_status = GeneratorStatus::STATUS_PRESENT;

    GeneratorModel expectedModel;
    expectedModel.model_detection = 0x06;
    expectedModel.generator_type = 0x08;

    ASSERT_EQ(generatorStateAndModel.first.time, expectedState.time);
    ASSERT_EQ(generatorStateAndModel.first.rotation_speed, expectedState.rotation_speed);
    ASSERT_EQ(generatorStateAndModel.first.start_battery_voltage, expectedState.start_battery_voltage);
    ASSERT_EQ(generatorStateAndModel.first.alarms, expectedState.alarms);
    ASSERT_EQ(generatorStateAndModel.first.start_signals, expectedState.start_signals);
    ASSERT_EQ(generatorStateAndModel.first.generator_status, expectedState.generator_status);

    ASSERT_EQ(generatorStateAndModel.second.model_detection, expectedModel.model_detection);
    ASSERT_EQ(generatorStateAndModel.second.generator_type, expectedModel.generator_type);
}

TEST_F(VariableSpeedMasterTest, it_parses_run_time_state) {
    driver.openURI("test://");

    std::vector<uint8_t> buffer = { variable_speed::PANELS_ADDRESS & 0xFF, (variable_speed::PANELS_ADDRESS >> 8) & 0xFF, variable_speed::DDC_CONTROLLER_ADDRESS & 0xFF,
                           (variable_speed::DDC_CONTROLLER_ADDRESS >> 8) & 0xFF, variable_speed::PACKET_RUN_TIME_STATE, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x44 };
    pushDataToDriver(&buffer[0], &buffer[16]);

    Frame frame = driver.readFrame();

    ASSERT_EQ(variable_speed::PACKET_RUN_TIME_STATE, frame.command);

    Time now = Time::now();
    RunTimeState runTimeState = driver.parseRunTimeState(frame.payload, now);

    RunTimeState expectedState;
    expectedState.time = now;
    expectedState.total_run_time = Time::fromSeconds((0x030201 * 60 * 60) + (0x00 * 60));
    expectedState.historical_run_time =  Time::fromSeconds((0x070605 * 60 * 60) + (0x04 * 60));

    ASSERT_EQ(runTimeState.time, expectedState.time);
    ASSERT_EQ(runTimeState.total_run_time, expectedState.total_run_time);
    ASSERT_EQ(runTimeState.historical_run_time, expectedState.historical_run_time);
    ASSERT_EQ(runTimeState.historical_run_time, expectedState.historical_run_time);
}
