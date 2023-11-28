#include <jsoncpp/json/json.h>
#include <fstream>
#include <genset_whisperpower_ddc/ExportParameters.hpp>
#include <iostream>

using namespace genset_whisperpower_ddc;

ExportParameters::ExportParameters()
{
}

ExportParameters::~ExportParameters()
{
}
void ExportParameters::exportParameters(pair<GeneratorState, GeneratorModel> genset,
    RunTimeState run_time_state)
{
    auto state = genset.first;
    auto model = genset.second;
    int default_space_delimited = 4;

    params["rotation speed"] = state.rotation_speed;
    params["start battery voltage"] = state.start_battery_voltage;
    params["alarms"] = state.alarms;
    params["start signal"] = state.start_signals;
    params["genset status"] = state.generator_status;
    params["model_detection"] = model.model_detection;
    params["generator type"] = model.generator_type;
    params["total run time"] = run_time_state.total_run_time;
    params["historical run time"] = run_time_state.historical_run_time;

    std::ifstream file("genset_parameters.json");
    if (file.good()) {
        // the file exists
        file.push_back(file);
    }
    else {
        // the file does not exist
        std::ofstream file("genset_parameters.json"); // create a file
        file << params.dump(default_space_delimited); // add info to the file
        file.close();
    }
}
