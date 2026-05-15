#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream> 
#include <algorithm> 
#include <iomanip> 
#include <windows.h> 
#include <tlhelp32.h> 
#include <fcntl.h> 
#include <io.h> 
using namespace std;
struct VM {
    map<string, int> variables;
    int program_counter = 0;
    map<string, int> labels;
    bool last_condition_passed = false;
    vector<bool> physical_ram = vector<bool>(8000000, false);
    vector<int> ram = vector<int>(10000, 0);
    vector<int> call_stack;
    HANDLE target_process_handle = NULL;
};
uintptr_t parseHex(const string& token) {
    uintptr_t address = 0;
    stringstream ss;
    if (token.rfind("0x", 0) == 0 || token.rfind("0X", 0) == 0) ss << hex << token.substr(2);
    else ss << hex << token;
    ss >> address;
    return address;
}
bool isMemoryAccess(const string& token) {
    return token.length() >= 3 && token.front() == '[' && token.back() == ']';
}
int getValue(const string& token, VM& vm) {
    if (isMemoryAccess(token)) {
        string inside = token.substr(1, token.length() - 2);
        int address = getValue(inside, vm);
        if (address >= 0 && address < (int)vm.ram.size()) return vm.ram[address];
        return 0;
    }
    try {
        size_t idx;
        int val = stoi(token, &idx);
        if (idx == token.length()) return val;
    }
    catch (...) {}
    return vm.variables[token];
}

void setValue(const string& target, int value, VM& vm) {
    if (isMemoryAccess(target)) {
        string inside = target.substr(1, target.length() - 2);
        int address = getValue(inside, vm);
        if (address >= 0 && address < (int)vm.ram.size()) vm.ram[address] = value;
    }
    else {
        try { stoi(target); return; }
        catch (...) {}
        vm.variables[target] = value;
    }
}
void dumpMemory(VM& vm, int start, int length) {
    cout << "\n--- [PHYSICAL RAM BINARY TABLE] ---" << endl;
    cout << "ADDR | 0 1 2 3 4 5 6 7 | HEX" << endl;
    cout << "-------------------------------" << endl;
    for (int i = start; i < start + length; i += 8) {
        if (i >= (int)vm.physical_ram.size()) break;
        cout << setw(4) << setfill('0') << i << " | ";
        int byte_val = 0;
        for (int j = 0; j < 8; ++j) {
            if (i + j < (int)vm.physical_ram.size()) {
                bool bit = vm.physical_ram[i + j];
                cout << (bit ? "1 " : "0 ");
                if (bit) byte_val |= (1 << (7 - j));
            }
        }
        cout << "| 0x" << hex << uppercase << byte_val << dec << endl;
    }
    cout << "-------------------------------\n" << endl;
}
vector<string> tokenize(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}
int executeLine(const string& current_line, VM& vm, int line_num) {
    string cleaned_line = current_line;
    cleaned_line.erase(remove(cleaned_line.begin(), cleaned_line.end(), '\r'), cleaned_line.end());
    replace(cleaned_line.begin(), cleaned_line.end(), '\t', ' ');
    vector<string> tokens = tokenize(cleaned_line);
    if (tokens.empty()) return 1;
    if (tokens[0][0] == ';' || tokens[0][0] == '#' || tokens[0][0] == ':') return 1;

    string cmd = tokens[0];
    if (cmd == "mov") {
        if (tokens.size() >= 3) {
            setValue(tokens[1], getValue(tokens[2], vm), vm);
        }
        else cout << "[ERROR]: mov target source" << endl;
    }
    else if (cmd == "add" || cmd == "sub" || cmd == "mul") {
        if (tokens.size() >= 4) {
            int v1 = getValue(tokens[2], vm);
            int v2 = getValue(tokens[3], vm);
            int res = (cmd == "add") ? v1 + v2 : (cmd == "sub") ? v1 - v2 : v1 * v2;
            setValue(tokens[1], res, vm);
        }
    }
    else if (cmd == "print") {
        size_t pos = cleaned_line.find("print") + 5;
        if (pos < cleaned_line.length()) {
            string text = cleaned_line.substr(pos);
            size_t first = text.find_first_not_of(' ');
            if (string::npos != first) text = text.substr(first);

            string output = "";
            for (size_t i = 0; i < text.length(); ++i) {
                if (text[i] == '%') {
                    size_t next = text.find('%', i + 1);
                    if (next != string::npos) {
                        string varName = text.substr(i + 1, next - i - 1);
                        output += to_string(getValue(varName, vm));
                        i = next;
                        continue;
                    }
                }
                output += text[i];
            }
            cout << output << endl;
        }
    }
    else if (cmd == "CLS") {
        printf("\x1b[H");
    }
    else if (cmd == "PXL") { // PXL <x> <y> <char_index>
        if (tokens.size() >= 4) {
            int x = getValue(tokens[1], vm);
            int y = getValue(tokens[2], vm);
            int c = getValue(tokens[3], vm);
            printf("\x1b[%d;%dH%c", y, x, (char)c);
        }
    }
    else if (cmd == "STB") {
        if (tokens.size() >= 3) {
            int addr = getValue(tokens[1], vm);
            int val = getValue(tokens[2], vm);
            if (addr >= 0 && addr < (int)vm.physical_ram.size()) {
                vm.physical_ram[addr] = (val != 0);
            }
            else cout << "[MEM ERROR]: Address " << addr << " out of range" << endl;
        }
    }
    else if (cmd == "GTB") {
        if (tokens.size() >= 3) {
            int addr = getValue(tokens[2], vm);
            if (addr >= 0 && addr < (int)vm.physical_ram.size()) {
                vm.variables[tokens[1]] = vm.physical_ram[addr] ? 1 : 0;
            }
        }
    }
    else if (cmd == "DMP") {
        if (tokens.size() >= 3) {
            dumpMemory(vm, getValue(tokens[1], vm), getValue(tokens[2], vm));
        }
    }
    else if (cmd == "goto") {
        if (tokens.size() >= 2 && vm.labels.count(tokens[1])) {
            vm.program_counter = vm.labels[tokens[1]];
            return 2;
        }
    }
    else if (cmd == "if") {
        if (tokens.size() >= 6 && tokens[4] == "goto") {
            int v1 = getValue(tokens[1], vm);
            string op = tokens[2];
            int v2 = getValue(tokens[3], vm);
            bool cond = false;
            if (op == "==") cond = (v1 == v2);
            else if (op == "!=") cond = (v1 != v2);
            else if (op == ">")  cond = (v1 > v2);
            else if (op == "<")  cond = (v1 < v2);

            if (cond && vm.labels.count(tokens[5])) {
                vm.program_counter = vm.labels[tokens[5]];
                return 2;
            }
        }
    }
    else if (cmd == "exit") {
        return 0;
    }
    else if (cmd == "add") {
        if (tokens.size() >= 4) setValue(tokens[1], getValue(tokens[2], vm) + getValue(tokens[3], vm), vm);
    }
    else if (cmd == "sub") {
        if (tokens.size() >= 4) setValue(tokens[1], getValue(tokens[2], vm) - getValue(tokens[3], vm), vm);
    }
    else if (cmd == "mul") {
        if (tokens.size() >= 4) setValue(tokens[1], getValue(tokens[2], vm) * getValue(tokens[3], vm), vm);
    }
    else if (cmd == "div") {
        if (tokens.size() >= 4) {
               int v2 = getValue(tokens[3], vm);
            if (v2 != 0) setValue(tokens[1], getValue(tokens[2], vm) / v2, vm);
        }
    }
    else if (cmd == "KEY") { // KEY <variable> <keycode>
        // keycode: 87=W, 65=A, 83=S, 68=D, 32=Space, 27=Esc
        int code = getValue(tokens[2], vm);
        vm.variables[tokens[1]] = (GetAsyncKeyState(code) & 0x8000) ? 1 : 0;
    }
    else if (cmd == "if") {
                            if (tokens.size() >= 6) {
                                int v1 = getValue(tokens[1], vm);
                                string op = tokens[2];
                                int v2 = getValue(tokens[3], vm);
                                bool res = false;
                                if (op == "==") res = (v1 == v2);
                                else if (op == "!=") res = (v1 != v2);
                                else if (op == ">")  res = (v1 > v2);
                                else if (op == "<")  res = (v1 < v2);

                                if (res && tokens[4] == "goto") {
                                    if (vm.labels.count(tokens[5])) {
                                        vm.program_counter = vm.labels[tokens[5]];
                                        return 2;
                                    }
                                }
                            }
                            }
    else if (cmd == "PXL") { // PXL x y char_code
        if (tokens.size() >= 4) {
                int x = getValue(tokens[1], vm);
                int y = getValue(tokens[2], vm);
                int c = getValue(tokens[3], vm);
                printf("\x1b[%d;%dH%c", y, x, (char)c);
            }
        }
    else if (cmd == "CLS") {
        printf("\x1b[H");
    }
    else if (cmd == "gosub") {
        if (tokens.size() >= 2 && vm.labels.count(tokens[1])) {
            vm.call_stack.push_back(vm.program_counter + 1);
            vm.program_counter = vm.labels[tokens[1]];
            return 2;
        }
    }
    else if (cmd == "return") {
        if (!vm.call_stack.empty()) {
            vm.program_counter = vm.call_stack.back();
            vm.call_stack.pop_back();
            return 2;
        }
    }
    else if (cmd == "SIN") { // SIN <res_var> <angle_deg> <scale>
        float rad = getValue(tokens[2], vm) * 3.14159f / 180.0f;
        setValue(tokens[1], (int)(sin(rad) * getValue(tokens[3], vm)), vm);
    }
    else if (cmd == "COS") { // COS <res_var> <angle_deg> <scale>
        float rad = getValue(tokens[2], vm) * 3.14159f / 180.0f;
        setValue(tokens[1], (int)(cos(rad) * getValue(tokens[3], vm)), vm);
    }
    else if (cmd == "SLEEP") {
                if (tokens.size() >= 2) Sleep(getValue(tokens[1], vm));
    }
    else if (cmd == "attach") {
        if (tokens.size() >= 2) {
            string target = tokens[1];
            DWORD pid = 0;
            if (target.find(".exe") != string::npos) {
                HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (snap != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32 pe; pe.dwSize = sizeof(pe);
                    if (Process32First(snap, &pe)) {
                        do {
                            wstring wName(pe.szExeFile);
                            string sName(wName.begin(), wName.end());
                            if (sName == target) { pid = pe.th32ProcessID; break; }
                        } while (Process32Next(snap, &pe));
                    }
                    CloseHandle(snap);
                }
            }
            else pid = (DWORD)getValue(target, vm);

            if (vm.target_process_handle) CloseHandle(vm.target_process_handle);
            vm.target_process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (vm.target_process_handle) cout << "[INFO]: Attached to " << target << " (PID: " << pid << ")" << endl;
            else cout << "[ERROR]: Attach failed. Code: " << GetLastError() << endl;
        }
    }
    else {
        if (!cmd.empty()) cout << "[DEBUG]: Unknown command: " << cmd << endl;
    }

    return 1; 
}
void runScript(const string& filename, VM& vm); // ♂♂♂♂♂♂♂♂♂♂♂♂
vector<string> loadLines(const string& filename) {
    ifstream f(filename);
    vector<string> lines;
    string l;
    while (getline(f, l)) lines.push_back(l);
    return lines;
}
void runScript(const string& filename, VM& vm) {
    vector<string> lines = loadLines(filename);
    if (lines.empty()) return;

    vm.labels.clear();
    for (int i = 0; i < (int)lines.size(); i++) {
        vector<string> t = tokenize(lines[i]);
        if (!t.empty() && t[0][0] == ':') vm.labels[t[0]] = i;
    }

    vm.program_counter = 0;
    while (vm.program_counter < (int)lines.size()) {
        int old_pc = vm.program_counter;
        int res = executeLine(lines[old_pc], vm, old_pc + 1);
        if (res == 0) break;
        if (res == 1) vm.program_counter++;
    }
}

int main(int argc, char* argv[]) {
    system("chcp 65001 > nul");
    setlocale(LC_ALL, ".UTF8");
    VM vm;
    if (argc >= 2) {
        string filename = argv[1];
        runScript(filename, vm);
        cin.get();
        return 0;
    }
    cout << "=== ASM+ 1.0.0 Realize ===" << endl;
    string input;
    while (true) {
        cout << "asm+ > ";
        if (!getline(cin, input) || input == "exit") break;
        if (input.rfind("run ", 0) == 0) {
            runScript(input.substr(4), vm);
        }
        else {
            executeLine(input, vm, 0);
        }
    }

    return 0;
}
//my tab got crushed, if you see some artifacts in tab, its normal, the my code without fixing tab:
/*
    else if (cmd == "if") {
                            if (tokens.size() >= 6) {
                                int v1 = getValue(tokens[1], vm);
                                string op = tokens[2];
                                int v2 = getValue(tokens[3], vm);
                                bool res = false;
                                if (op == "==") res = (v1 == v2);
                                else if (op == "!=") res = (v1 != v2);
                                else if (op == ">")  res = (v1 > v2);
                                else if (op == "<")  res = (v1 < v2);

                                if (res && tokens[4] == "goto") {
                                    if (vm.labels.count(tokens[5])) {
                                        vm.program_counter = vm.labels[tokens[5]];
                                        return 2;
                                    }
                                }
                            }
                            }
*/