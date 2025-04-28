#include <genset_whisperpower_ddc/GeneratorState.hpp>
#include <gtest/gtest.h>

using namespace genset_whisperpower_ddc;

struct GeneratorStateTest : public ::testing::Test {};

TEST_F(GeneratorStateTest, it_converts_to_the_common_GensetState_type)
{
    GeneratorState state;
    state.time = base::Time::fromSeconds(10);
    state.generator_status = STATUS_PRE_GLOW;

    auto converted = state.toGensetState();
    ASSERT_EQ(state.time, converted.time);
    ASSERT_FALSE(converted.failure_detected);
    ASSERT_EQ(power_whisperpower::GensetState::GENSET_STAGE_STARTING, converted.stage);
}

TEST_F(GeneratorStateTest, it_fills_the_failure_detected_field)
{
    GeneratorState state;
    state.time = base::Time::fromSeconds(10);
    state.alarms = 10;
    state.generator_status = STATUS_PRE_GLOW;

    auto converted = state.toGensetState();
    ASSERT_EQ(state.time, converted.time);
    ASSERT_TRUE(converted.failure_detected);
    ASSERT_EQ(power_whisperpower::GensetState::GENSET_STAGE_STARTING, converted.stage);
}