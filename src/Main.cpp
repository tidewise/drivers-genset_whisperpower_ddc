#include <iostream>
#include <list>
#include <memory>
#include <genset_whisperpower_ddc/VariableSpeed.hpp>
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

    std::unique_ptr<VariableSpeedMaster> master(new VariableSpeedMaster());
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

        master->sendControlCommand(
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

        Frame frame;
        try
        {
            frame = master->readFrame();
        }
        catch(const variable_speed::WrongSize& e)
        {
            cerr << e.what() << "\n";
            return 1;
        }
        catch(const variable_speed::InvalidChecksum& e)
        {
            cerr << e.what() << "\n";
            return 1;
        }

        if (frame.targetID != variable_speed::TARGET_ADDRESS) {
            cerr << "unknown target ID '0x" <<  std::hex << (uint16_t) frame.targetID << "'\n\n";
            return 1;
        }

        if (frame.sourceID != variable_speed::SOURCE_ADDRESS) {
            cerr << "unknown source ID '0x" <<  std::hex << (uint16_t) frame.sourceID << "'\n\n";
            return 1;
        }

        if (frame.command == variable_speed::PACKET_GENERATOR_STATE_AND_MODEL){
            std::pair<GeneratorState, GeneratorModel> generatorStateAndModel = master->parseGeneratorStateAndModel(frame.payload, now);

            cout << generatorStateAndModel.first << endl;
            cout << generatorStateAndModel.second << endl;
        }
        else if (frame.command == variable_speed::PACKET_RUN_TIME_STATE) {
            RunTimeState runTimeState = master->parseRunTimeState(frame.payload, now);

            cout << runTimeState << endl;
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
