#include <jsoncpp/json/json.h>
#include <base/Time.hpp>
#include <fstream>
#include <genset_whisperpower_ddc/ControlCommand.hpp>
#include <genset_whisperpower_ddc/GeneratorModel.hpp>
#include <genset_whisperpower_ddc/GeneratorState.hpp>
#include <genset_whisperpower_ddc/RunTimeState.hpp>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <signal.h>

using namespace std;
using namespace genset_whisperpower_ddc;

void writeAndExportParameters(pair<GeneratorState, GeneratorModel> genset,
    RunTimeState run_time_state)
{
    Json::Value params;
    GeneratorState state = genset.first;
    GeneratorModel model = genset.second;

    params["state"]["rotation speed"] = state.rotation_speed;
    params["state"]["start battery voltage"] = state.start_battery_voltage;
    params["state"]["alarms"] = state.alarms;
    params["state"]["start signal"] = state.start_signals;
    params["state"]["genset status"] = state.generator_status;
    params["model"]["model_detection"] = model.model_detection;
    params["model"]["generator type"] = model.generator_type;
    params["run time state"]["total run time"] =
        (int)run_time_state.total_run_time.toMilliseconds();
    params["run time state"]["historical run time"] =
        (int)run_time_state.historical_run_time.toMilliseconds();

    fstream file("genset_parameters.json");
    Json::StreamWriterBuilder builder;
    unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(params, &file);
}

bool stop = false;

void usage(ostream& stream)
{
    stream << "usage: genset_whisperpower_ddc_ctl URI CMD\n"
           << "where:\n"
           << "    URI is a iodrivers_base URI\n"
           << "\n"
           << "Available Commands\n"
           << "    run: send start control command to the DDC GenVerter power controller "
              "and then keep it running by "
           << "repeatedly sending the keep_alive command until the user press any key. "
              "When this happens, send the stop command to stop de generator.\n"
           << endl;
}

void handler(int s)
{
    stop = true;
}

Frame waitForValidFrame(unique_ptr<VariableSpeedMaster>& master)
{
    while (true) {
        try {
            Frame frame = master->readFrame();
            if (frame.targetID == variable_speed::PANELS_ADDRESS &&
                frame.sourceID == variable_speed::DDC_CONTROLLER_ADDRESS) {
                return frame;
            }
        }
        catch (const variable_speed::WrongSize& e) {
        }
        catch (const variable_speed::InvalidChecksum& e) {
        }
        // iodrivers_base may throw this error when receiving a SIGINT, but it can be
        // ignored
        catch (const iodrivers_base::UnixError& e) {
        }
    }
    // Never reached
}

int main(int argc, char** argv)
{
    list<string> args(argv + 1, argv + argc);
    if (args.size() < 2) {
        bool error = !args.empty();
        usage(error ? cerr : cout);
        return error;
    }
    string uri = args.front();
    args.pop_front();

    if (args.empty()) {
        usage(cerr);
        return 1;
    }

    string cmd = args.front();
    args.pop_front();

    unique_ptr<VariableSpeedMaster> master(new VariableSpeedMaster());
    master->openURI(uri);

    if (cmd == "run") {
        if (args.size() != 0) {
            cerr << "command run does not accept arguments\n\n";
            usage(cerr);
            return 1;
        }

        waitForValidFrame(master);

        master->sendControlCommand(CONTROL_CMD_START);

        signal(SIGINT, handler);

        // Send keep_alive until user presses Ctrl+C
        while (!stop) {
            waitForValidFrame(master);

            try {
                master->sendControlCommand(CONTROL_CMD_KEEP_ALIVE);
            }
            catch (const iodrivers_base::UnixError& e) {
            } // iodrivers_base may throw this error when receiving a SIGINT, but it can
              // be ignored
        }

        waitForValidFrame(master);

        Frame frame;
        base::Time now;
        pair<GeneratorState, GeneratorModel> generatorStateAndModel;
        RunTimeState runTimeState;
        bool receivedGeneratorState = false;
        bool receivedRunTimeState = false;

        while (!(receivedGeneratorState && receivedRunTimeState)) {
            frame = waitForValidFrame(master);

            now = base::Time::now();
            if (frame.command == variable_speed::PACKET_GENERATOR_STATE_AND_MODEL) {
                generatorStateAndModel =
                    master->parseGeneratorStateAndModel(frame.payload, now);
                receivedGeneratorState = true;
            }
            else if (frame.command == variable_speed::PACKET_RUN_TIME_STATE) {
                runTimeState = master->parseRunTimeState(frame.payload, now);
                receivedRunTimeState = true;
            }
        }

        writeAndExportParameters(generatorStateAndModel, runTimeState);
        cout << generatorStateAndModel.first << endl;
        cout << generatorStateAndModel.second << endl;
        cout << runTimeState << endl;

        master->sendControlCommand(CONTROL_CMD_STOP);
    }

    else {
        cerr << "unknown command '" << cmd << "'\n\n";
        usage(cerr);
        return 1;
    }
    return 0;
}
