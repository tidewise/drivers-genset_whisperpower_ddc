#include <iostream>
#include <list>
#include <memory>
#include <signal.h>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/ControlCommand.hpp>

using namespace std;
using namespace genset_whisperpower_ddc;

bool stop = false;

void usage(ostream& stream) {
    stream << "usage: genset_whisperpower_ddc_ctl URI CMD\n"
           << "where:\n"
           << "    URI is a iodrivers_base URI\n"
           << "\n"
           << "Available Commands\n"
           << "    run: send start control command to the DDC GenVerter power controller and then keep it running by "
           << "repeatedly sending the keep_alive command until the user press any key. When this happens, send the stop command to stop de generator.\n"
           << "    read_frame: read a frame and print its content\n"
           << endl;
}

void handler(int s) {
    stop = true;
}

void getValidFrame(std::unique_ptr<VariableSpeedMaster> &master) {
    Frame frame;
    bool validFrame = false;
    bool receivedValidFrame = false;

    while (!receivedValidFrame) {
        try {
            frame = master->readFrame();
            validFrame = true;
        }
        catch(const variable_speed::WrongSize& e) {
            validFrame = false;
        }
        catch(const variable_speed::InvalidChecksum& e) {
            validFrame = false;
        }
        // iodrivers_base may throw this error when receiving a SIGINT, but it can be ignored
        catch(const iodrivers_base::UnixError& e) {
            validFrame = false;
        }

        if (validFrame) {
            if (frame.targetID == variable_speed::PANELS_ADDRESS && frame.sourceID == variable_speed::DDC_CONTROLLER_ADDRESS) {
                receivedValidFrame = true;
            }
        }
    }
}

int main(int argc, char** argv) {
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

    std::unique_ptr<VariableSpeedMaster> master(new VariableSpeedMaster());
    master->openURI(uri);

    if (cmd == "run") {
        if (args.size() != 0) {
            cerr << "command run does not accept arguments\n\n";
            usage(cerr);
            return 1;
        }

        getValidFrame(master);

        master->sendControlCommand(CONTROL_CMD_START);

        signal(SIGINT, handler);

        // Send keep_alive until user presses Ctrl+C
        while(!stop) {
            getValidFrame(master);

            try {
                master->sendControlCommand(CONTROL_CMD_KEEP_ALIVE);
            }
            catch(const iodrivers_base::UnixError& e) {} // iodrivers_base may throw this error when receiving a SIGINT, but it can be ignored
        }

        getValidFrame(master);

        master->sendControlCommand(CONTROL_CMD_STOP);
    }
    else if (cmd == "read_frame") {
        if (args.size() != 0) {
                cerr << "command read-frame does not accept arguments\n\n";
                usage(cerr);
                return 1;
        }

        base::Time now;

        Frame frame;
        bool validFrame;
        std::pair<GeneratorState, GeneratorModel> generatorStateAndModel;
        RunTimeState runTimeState;
        bool receivedGeneratorState = false;
        bool receivedRunTimeState = false;
        while (!(receivedGeneratorState && receivedRunTimeState)) {
            try {
                frame = master->readFrame();
                validFrame = true;
            }
            catch(const variable_speed::WrongSize& e) {
                validFrame = false;
            }
            catch(const variable_speed::InvalidChecksum& e) {
                validFrame = false;
            }
            if (validFrame) {
                if (frame.targetID == variable_speed::PANELS_ADDRESS && frame.sourceID == variable_speed::DDC_CONTROLLER_ADDRESS) {
                    now = base::Time::now();
                    if (frame.command == variable_speed::PACKET_GENERATOR_STATE_AND_MODEL){
                        generatorStateAndModel = master->parseGeneratorStateAndModel(frame.payload, now);
                        receivedGeneratorState = true;
                    }
                    else if (frame.command == variable_speed::PACKET_RUN_TIME_STATE) {
                        runTimeState = master->parseRunTimeState(frame.payload, now);
                        receivedRunTimeState = true;
                    }
                }
            }
        }

        cout << generatorStateAndModel.first << endl;
        cout << generatorStateAndModel.second << endl;
        cout << runTimeState << endl;
    }
    else {
        cerr << "unknown command '" << cmd << "'\n\n";
        usage(cerr);
        return 1;
    }
    return 0;
}
