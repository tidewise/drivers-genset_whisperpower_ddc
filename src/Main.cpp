#include <iostream>
#include <list>
#include <memory>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/GeneratorStatus.hpp>
#include <genset_whisperpower_ddc/ControlCommand.hpp>
#include <bits/stdc++.h>

using namespace std;
using namespace genset_whisperpower_ddc;

#define STATUS_CASE(name) case STATUS_##name: return #name;
std::string statusToString(GeneratorStatus status) {
    switch (status) {
        STATUS_CASE(NONE)
        STATUS_CASE(PRE_GLOW)
        STATUS_CASE(GLOW)
        STATUS_CASE(START)
        STATUS_CASE(SWITCH)
        STATUS_CASE(STOP)
        STATUS_CASE(FAILURE)
        STATUS_CASE(PRESENT)
        STATUS_CASE(FIRST)
        STATUS_CASE(ERROR_BYPASS)
        STATUS_CASE(PRE_ERROR_BYPASS)
        STATUS_CASE(PUMPING)
        STATUS_CASE(RESTART)
        STATUS_CASE(HYDROBOOST)
        STATUS_CASE(UNKNOWN)
    }
    return "UNKNOWN";
}

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
            controlCommand = ControlCommand::start;
        }
        else if (controlCommandArg == "stop") {
            controlCommand = ControlCommand::stop;
        }
        else if (controlCommandArg == "keep_alive") {
            controlCommand = ControlCommand::keepAlive;
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
    else if (cmd == "read-frame") {
        if (args.size() != 0) {
                cerr << "command read-frame does not accept arguments\n\n";
                usage(cerr);
                return 1;
        }

        Frame frame = genset_whisperpower_ddc_master->readFrame();

        if (frame.targetID != 0x0081) {
            cerr << "unknown target ID '0x" <<  std::hex << (uint16_t) frame.targetID << "'\n\n";
            return 1;
        }

        if (frame.sourceID != 0x0088) {
            cerr << "unknown source ID '0x" <<  std::hex << (uint16_t) frame.sourceID << "'\n\n";
            return 1;
        }

        if (frame.command == 2){
            int rpm = (frame.payload[1] << 8) | frame.payload[0];
            int udcStartBattery = (frame.payload[3] << 8) | frame.payload[2];
            std::bitset<8> statusA(frame.payload[4]);
            bool overallAlarm = statusA[0];
            bool engineTempAlarm = statusA[1];
            bool pmVoltageAlarm = statusA[2];
            bool oilPressAlarm = statusA[3];
            bool exhaustTempAlarm = statusA[4];
            bool uac1Alarm = statusA[5];
            bool iac1Alarm = statusA[6];
            bool oilPressHighAlarm = statusA[7];
            std::bitset<8> statusB(frame.payload[5]);
            bool lowStartBatteryVoltAlarm = statusB[2];
            bool startFailure = statusB[4];
            bool runSignal = statusB[5];
            bool startByOpUnit = statusB[7];
            std::bitset<8> statusC(frame.payload[6]);
            bool modelDetection50Hz = statusC[2];
            bool modelDetection60Hz = statusC[3];
            bool modelDetection3Phase = statusC[4];
            bool modelDetectionMobile = statusC[5];
            GeneratorStatus generatorStatus;
            if (frame.payload[7] < 0x0E){
                generatorStatus = static_cast<GeneratorStatus>(frame.payload[7]);
            }
            else{
                generatorStatus = STATUS_UNKNOWN;
            }
            int generatorType = frame.payload[8];

            cout << "RPM: " << std::dec << rpm << endl;
            cout << "Udc start battery: " << std::dec << udcStartBattery * 0.01 << "V" << endl;
            cout << "Alarms:" << endl;
            if (overallAlarm){
                cout << "    overall" << endl;
            }
            if (engineTempAlarm){
                cout << "    engine temperature" << endl;
            }
            if (pmVoltageAlarm){
                cout << "    PM voltage" << endl;
            }
            if (oilPressAlarm){
                cout << "    oil pressure" << endl;
            }
            if (exhaustTempAlarm){
                cout << "    exhaust temperature" << endl;
            }
            if (uac1Alarm){
                cout << "    Uac1" << endl;
            }
            if (iac1Alarm){
                cout << "    Iac1" << endl;
            }
            if (oilPressHighAlarm){
                cout << "    oil pressure high" << endl;
            }
            if (lowStartBatteryVoltAlarm){
                cout << "    low start battery voltage" << endl;
            }
            cout << "start failure: " << (startFailure ? "true" : "false") << endl;
            cout << "run signal: " << (runSignal ? "true" : "false") << endl;
            cout << "start by operation unit: " << (startByOpUnit ? "true" : "false") << endl;
            cout << "Model detection:" << endl;
            if (modelDetection50Hz){
                cout << "    50 Hz model detection" << endl;
            }
            if (modelDetection60Hz){
                cout << "    60 Hz model detection" << endl;
            }
            if (modelDetection3Phase){
                cout << "    3 phase model detection" << endl;
            }
            if (modelDetectionMobile){
                cout << "    mobile model detection" << endl;
            }
            cout << "generator status: " << statusToString(generatorStatus)  << endl;
            cout << "generator type: " << std::dec << generatorType << endl;
            cout << endl;
        }
        else if (frame.command == 14) {
            int totalMinutes = frame.payload[0];
            int totalHours = (frame.payload[3] << 16) | (frame.payload[2] << 8) | frame.payload[1];
            int historicalMinutes = frame.payload[4];
            int historicalHours = (frame.payload[7] << 16) | (frame.payload[6] << 8) | frame.payload[5];

            cout << "total run time to be reset after maintenance: " << std::dec << totalHours << "h" << totalMinutes << "min" << endl;
            cout << "historical run time: " << std::dec << historicalHours << "h" << historicalMinutes << "min" << endl;
            cout << endl;
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
