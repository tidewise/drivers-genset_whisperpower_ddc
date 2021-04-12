#include <iostream>
#include <list>
#include <memory>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <genset_whisperpower_ddc/GeneratorStatus.hpp>
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

struct Command02Data {
    int rpm;
    int udcStartBattery;
    bool overallAlarm;
    bool engineTempAlarm;
    bool pmVoltageAlarm;
    bool oilPressAlarm;
    bool exhaustTempAlarm;
    bool uac1Alarm;
    bool iac1Alarm;
    bool oilPressHighAlarm;
    bool lowStartBatteryVoltAlarm;
    bool startFailure;
    bool runSignal;
    bool startByOpUnit;
    bool modelDetection50Hz;
    bool modelDetection60Hz;
    bool modelDetection3Phase;
    bool modelDetectionMobile;
    GeneratorStatus generatorStatus;
    int generatorType;
};

struct Command14Data
{
    int totalMinutes;
    int totalHours;
    int historicalMinutes;
    int historicalHours;
};



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

        string control_command_arg = args.front();
        int control_command;

        if (control_command_arg == "start") {
            control_command = 1;
        }
        else if (control_command_arg == "stop") {
            control_command = 2;
        }
        else if (control_command_arg == "keep_alive") {
            control_command = 3;
        }
        else {
            cerr << "unknown control command\n\n";
            usage(cerr);
            return 1;
        }

        args.pop_front();

        genset_whisperpower_ddc_master->sendCommandF7(
            control_command
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
            Command02Data data;
            data.rpm = (frame.payload[1] << 8) | frame.payload[0];
            data.udcStartBattery = (frame.payload[3] << 8) | frame.payload[2];
            std::bitset<8> statusA(frame.payload[4]);
            data.overallAlarm = statusA[0];
            data.engineTempAlarm = statusA[1];
            data.pmVoltageAlarm = statusA[2];
            data.oilPressAlarm = statusA[3];
            data.exhaustTempAlarm = statusA[4];
            data.uac1Alarm = statusA[5];
            data.iac1Alarm = statusA[6];
            data.oilPressHighAlarm = statusA[7];
            std::bitset<8> statusB(frame.payload[5]);
            data.lowStartBatteryVoltAlarm = statusB[2];
            data.startFailure = statusB[4];
            data.runSignal = statusB[5];
            data.startByOpUnit = statusB[7];
            std::bitset<8> statusC(frame.payload[6]);
            data.modelDetection50Hz = statusC[2];
            data.modelDetection60Hz = statusC[3];
            data.modelDetection3Phase = statusC[4];
            data.modelDetectionMobile = statusC[5];
            if (frame.payload[7] < 0x0E){
                data.generatorStatus = static_cast<GeneratorStatus>(frame.payload[7]);
            }
            else{
                data.generatorStatus = STATUS_UNKNOWN;
            }
            data.generatorType = frame.payload[8];

            cout << "RPM: " << std::dec << data.rpm << endl;
            cout << "Udc start battery: " << std::dec << data.udcStartBattery * 0.01 << "V" << endl;
            cout << "Alarms:" << endl;
            if (data.overallAlarm){
                cout << "    overall" << endl;
            }
            if (data.engineTempAlarm){
                cout << "    engine temperature" << endl;
            }
            if (data.pmVoltageAlarm){
                cout << "    PM voltage" << endl;
            }
            if (data.oilPressAlarm){
                cout << "    oil pressure" << endl;
            }
            if (data.exhaustTempAlarm){
                cout << "    exhaust temperature" << endl;
            }
            if (data.uac1Alarm){
                cout << "    Uac1" << endl;
            }
            if (data.iac1Alarm){
                cout << "    Iac1" << endl;
            }
            if (data.oilPressHighAlarm){
                cout << "    oil pressure high" << endl;
            }
            if (data.lowStartBatteryVoltAlarm){
                cout << "    low start battery voltage" << endl;
            }
            cout << "start failure: " << (data.startFailure ? "true" : "false") << endl;
            cout << "run signal: " << (data.runSignal ? "true" : "false") << endl;
            cout << "start by operation unit: " << (data.startByOpUnit ? "true" : "false") << endl;
            cout << "Model detection:" << endl;
            if (data.modelDetection50Hz){
                cout << "    50 Hz model detection" << endl;
            }
            if (data.modelDetection60Hz){
                cout << "    60 Hz model detection" << endl;
            }
            if (data.modelDetection3Phase){
                cout << "    3 phase model detection" << endl;
            }
            if (data.modelDetectionMobile){
                cout << "    mobile model detection" << endl;
            }
            cout << "generator status: " << statusToString(data.generatorStatus)  << endl;
            cout << "generator type: " << std::dec << data.generatorType << endl;
            cout << endl;
        }
        else if (frame.command == 14) {
            Command14Data data;
            data.totalMinutes = frame.payload[0];
            data.totalHours = (frame.payload[3] << 16) | (frame.payload[2] << 8) | frame.payload[1];
            data.historicalMinutes = frame.payload[4];
            data.historicalHours = (frame.payload[7] << 16) | (frame.payload[6] << 8) | frame.payload[5];

            cout << "total run time to be reset after maintenance: " << std::dec << data.totalHours << "h" << data.totalMinutes << "min" << endl;
            cout << "historical run time: " << std::dec << data.historicalHours << "h" << data.historicalMinutes << "min" << endl;
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
