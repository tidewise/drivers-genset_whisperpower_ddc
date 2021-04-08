#include <iostream>
#include <list>
#include <memory>
#include <genset_whisperpower_ddc/VariableSpeedMaster.hpp>
#include <bits/stdc++.h>

using namespace std;
using namespace genset_whisperpower_ddc;

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
    bool mdlDetection50Hz;
    bool mdlDetection60Hz;
    bool mdlDetection3Phase;
    bool mobileMdlDetection;
    string generatorStatus;
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
           << "  send_control_command CONTROL_COMMAND: send command F7 frame containing a control command\n"
           << "    CONTROL_COMMAND can be start, stop or keep_alive\n"
           << "  read_frame [--verbose]\n"
           << "    --verbose: print the read frames attributes\n"
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
        if (args.size() > 1) {
            cerr << "too many arguments\n\n";
            usage(cerr);
            return 1;
        }

        bool verbose = false;
        if (args.size() == 1) {
            if (args.front() == "--verbose") {
                verbose = true;
                args.pop_front();
            }
            else {
                cerr << "unknown argument '" << args.front() << "'\n\n";
                usage(cerr);
                return 1;
            }
        }

        string target;
        string source;

        cout << "Listening... Press Ctrl+C to stop" << endl;
        while(1) {
            Frame frame = genset_whisperpower_ddc_master->readFrame();

            if (frame.targetID == 0x0081) {
                target = "monitor panels";
            }
            else {
                cerr << "unknown target ID '0x" <<  std::hex << (uint16_t) frame.targetID << "'\n\n";
                return 1;
            }

            if (frame.sourceID == 0x0088) {
                source = "DDC GenVerter power controller";
            }
            else {
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
                data.mdlDetection50Hz = statusC[2];
                data.mdlDetection60Hz = statusC[3];
                data.mdlDetection3Phase = statusC[4];
                data.mobileMdlDetection = statusC[5];
                if (frame.payload[7] == 0x00){
                    data.generatorStatus = "None";
                }
                else if (frame.payload[7] == 0x01){
                    data.generatorStatus = "PreGlow";
                }
                else if (frame.payload[7] == 0x02){
                    data.generatorStatus = "Glow";
                }
                else if (frame.payload[7] == 0x03){
                    data.generatorStatus = "Start";
                }
                else if (frame.payload[7] == 0x04){
                    data.generatorStatus = "Switch";
                }
                else if (frame.payload[7] == 0x05){
                    data.generatorStatus = "Stop";
                }
                else if (frame.payload[7] == 0x06){
                    data.generatorStatus = "Failure";
                }
                else if (frame.payload[7] == 0x07){
                    data.generatorStatus = "Present";
                }
                else if (frame.payload[7] == 0x08){
                    data.generatorStatus = "First";
                }
                else if (frame.payload[7] == 0x09){
                    data.generatorStatus = "ErrorBypass";
                }
                else if (frame.payload[7] == 0x0A){
                    data.generatorStatus = "PreErrorBypass";
                }
                else if (frame.payload[7] == 0x0B){
                    data.generatorStatus = "Pumping";
                }
                else if (frame.payload[7] == 0x0C){
                    data.generatorStatus = "Restart";
                }
                else if (frame.payload[7] == 0x0D){
                    data.generatorStatus = "HydroBoost";
                }
                else{
                    cerr << "unknown generator status '0x" <<  std::hex << (uint16_t) frame.payload[7] << "'\n\n";
                    return 1;
                }
                data.generatorType = frame.payload[8];

                if (verbose) {
                    cout << "target: " << target << endl;
                    cout << "source: " << source << endl;
                    cout << "command: " << (int) frame.command << endl;
                    cout << "command: 0x" << std::hex << (uint16_t) frame.command << endl;
                    cout << "RPM: " << data.rpm << endl;
                    cout << "Udc start battery: " << data.udcStartBattery * 0.01 << "V" << endl;
                    cout << "overall alarm: " << (data.overallAlarm ? "true" : "false") << endl;
                    cout << "engine temperature alarm: " << (data.engineTempAlarm ? "true" : "false") << endl;
                    cout << "PM voltage alarm: " << (data.pmVoltageAlarm ? "true" : "false") << endl;
                    cout << "oil pressure alarm: " << (data.oilPressAlarm ? "true" : "false") << endl;
                    cout << "exhaust temperature alarm: " << (data.exhaustTempAlarm ? "true" : "false") << endl;
                    cout << "Uac1 alarm: " << (data.uac1Alarm ? "true" : "false") << endl;
                    cout << "Iac1 alarm: " << (data.iac1Alarm ? "true" : "false") << endl;
                    cout << "oil pressure high alarm: " << (data.oilPressHighAlarm ? "true" : "false") << endl;
                    cout << "low start battery voltage alarm: " << (data.lowStartBatteryVoltAlarm ? "true" : "false") << endl;
                    cout << "start failure: " << (data.startFailure ? "true" : "false") << endl;
                    cout << "run signal: " << (data.runSignal ? "true" : "false") << endl;
                    cout << "start by operation unit: " << (data.startByOpUnit ? "true" : "false") << endl;
                    cout << "50 Hz model detection: " << (data.mdlDetection50Hz ? "true" : "false") << endl;
                    cout << "60 Hz model detection: " << (data.mdlDetection60Hz ? "true" : "false") << endl;
                    cout << "3 phase model detection: " << (data.mdlDetection3Phase ? "true" : "false") << endl;
                    cout << "mobile model detection: " << (data.mobileMdlDetection ? "true" : "false") << endl;
                    cout << "generator status: " << data.generatorStatus << endl;
                    cout << "generator type: " << data.generatorType << endl;
                    cout << endl;
                }
            }
            else if (frame.command == 14) {
                Command14Data data;
                data.totalMinutes = frame.payload[0];
                data.totalHours = (frame.payload[3] << 16) | (frame.payload[2] << 8) | frame.payload[1];
                data.historicalMinutes = frame.payload[4];
                data.historicalHours = (frame.payload[7] << 16) | (frame.payload[6] << 8) | frame.payload[5];

                if (verbose) {
                    cout << "target: " << target << endl;
                    cout << "source: " << source << endl;
                    cout << "command: " << (int) frame.command << endl;
                    cout << "command: 0x" << std::hex << (uint16_t) frame.command << endl;
                    cout << "total run time to be reset after maintenance: " << data.totalHours << "h" << data.totalMinutes << "min" << endl;
                    cout << "historical run time: " << data.historicalHours << "h" << data.historicalMinutes << "min" << endl;
                    cout << endl;
                }
            }
            else {
                cout << "target: " << target << endl;
                cout << "source: " << source << endl;
                cout << "command: " << frame.command << endl;
            }
        }
    }
    else {
        cerr << "unknown command '" << cmd << "'\n\n";
        usage(cerr);
        return 1;
    }
    return 0;
}
