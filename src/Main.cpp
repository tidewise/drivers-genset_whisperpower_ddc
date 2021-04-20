#include <iostream>
#include <list>
#include <memory>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/ControlCommand.hpp>

using namespace std;
using namespace genset_whisperpower_ddc;

void usage(ostream& stream) {
    stream << "usage: genset_whisperpower_ddc_ctl URI CMD\n"
           << "where:\n"
           << "  URI is a iodrivers_base URI\n"
           << "\n"
           << "Available Commands\n"
           << "  send_control_command CONTROL_COMMAND: send a control command to the DDC GenVerter power controller\n"
           << "    CONTROL_COMMAND can be start, stop or keep_alive\n"
           << "  read_frame: read a frame and print its content\n"
           << endl;
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

    std::unique_ptr<genset_whisperpower_ddc::VariableSpeedMaster> genset_whisperpower_ddc_master;
    auto* master = new genset_whisperpower_ddc::VariableSpeedMaster();
    genset_whisperpower_ddc_master.reset(master);
    master->openURI(uri);

    if (cmd == "send_control_command") {
        if (args.size() != 1) {
            cerr << "wrong number of arguments\n\n";
            usage(cerr);
            return 1;
        }

        string controlCommandArg = args.front();
        ControlCommand controlCommand;

        if (controlCommandArg == "start") {
            controlCommand = CONTROL_CMD_START;
        }
        else if (controlCommandArg == "stop") {
            controlCommand = CONTROL_CMD_STOP;
        }
        else if (controlCommandArg == "keep_alive") {
            controlCommand = CONTROL_CMD_KEEP_ALIVE;
        }
        else {
            cerr << "unknown control command\n\n";
            usage(cerr);
            return 1;
        }

        args.pop_front();

        genset_whisperpower_ddc_master->sendControlCommand(
            controlCommand
        );
    }
    else if (cmd == "read_frame") {
        if (args.size() != 0) {
                cerr << "command read-frame does not accept arguments\n\n";
                usage(cerr);
                return 1;
        }

        auto now = base::Time::now();
        Frame frame;// = genset_whisperpower_ddc_master->readFrame();
        frame.targetID = 0x0081;
        frame.sourceID = 0x0088;
        frame.command = 14;

        if (frame.targetID != 0x0081) {
            cerr << "unknown target ID '0x" <<  std::hex << (uint16_t) frame.targetID << "'\n\n";
            return 1;
        }

        if (frame.sourceID != 0x0088) {
            cerr << "unknown source ID '0x" <<  std::hex << (uint16_t) frame.sourceID << "'\n\n";
            return 1;
        }

        if (frame.command == 2){
            GeneratorState generatorState = genset_whisperpower_ddc_master->parseGeneratorState(frame.payload, now);

            cout << generatorState << endl;
        }
        else if (frame.command == 14) {
            RuntimeState runtimeState = genset_whisperpower_ddc_master->parseRuntimeState(frame.payload, now);

            cout << runtimeState << endl;
        }
        else {
            cerr << "unknown command '0x" <<  std::hex << (uint16_t) frame.command << "'\n\n";
            return 1;
        }

    }
    else {
        cerr << "unknown command '" << cmd << "'\n\n";
        usage(cerr);
        return 1;
    }
    return 0;
}
