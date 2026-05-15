#include "commands.h"
#include "memory_control.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <tlhelp32.h>

int executeLine(const std::string& current_line, VM& vm) {
    std::string line = current_line;
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    std::vector<std::string> tokens = tokenize(line);

    if (tokens.empty() || tokens[0][0] == ';' || tokens[0][0] == ':') return 1;

    std::string cmd = tokens[0];

    if (cmd == "mov") {
        if (tokens.size() >= 3) setValue(tokens[1], getValue(tokens[2], vm), vm);
    }
    else if (cmd == "add" || cmd == "sub" || cmd == "mul" || cmd == "div") {
        if (tokens.size() >= 4) {
            int v1 = getValue(tokens[2], vm);
            int v2 = getValue(tokens[3], vm);
            int res = 0;
            if (cmd == "add") res = v1 + v2;
            else if (cmd == "sub") res = v1 - v2;
            else if (cmd == "mul") res = v1 * v2;
            else if (cmd == "div" && v2 != 0) res = v1 / v2;
            setValue(tokens[1], res, vm);
        }
    }
    else if (cmd == "print") {
        std::string output = "";
        size_t pos = line.find("print") + 5;
        std::string text = line.substr(pos);
        for (size_t i = 0; i < text.length(); ++i) {
            if (text[i] == '%') {
                size_t next = text.find('%', i + 1);
                if (next != std::string::npos) {
                    output += std::to_string(getValue(text.substr(i + 1, next - i - 1), vm));
                    i = next; continue;
                }
            }
            output += text[i];
        }
        std::cout << output << std::endl;
    }
    else if (cmd == "run") {
        if (tokens.size() >= 2) {
            runScript(tokens[1], vm);
        }
        else {
            std::cout << "[Error]: Usage: run <filename.asmp>" << std::endl;
        }
    }
    else if (cmd == "PXL") {
        if (tokens.size() >= 4)
            printf("\x1b[%d;%dH%c", getValue(tokens[2], vm), getValue(tokens[1], vm), (char)getValue(tokens[3], vm));
    }
    else if (cmd == "CLS") {
        // \x1b[2J очистка \x1b[H киоск
        printf("\x1b[2J\x1b[H");
    }
    else if (cmd == "SLEEP") Sleep(getValue(tokens[1], vm));
    else if (cmd == "STB") {
        int addr = getValue(tokens[1], vm);
        if (addr >= 0 && addr < (int)vm.physical_ram.size()) vm.physical_ram[addr] = (getValue(tokens[2], vm) != 0);
    }
    else if (cmd == "GTB") {
        int addr = getValue(tokens[2], vm);
        if (addr >= 0 && addr < (int)vm.physical_ram.size()) vm.variables[tokens[1]] = vm.physical_ram[addr] ? 1 : 0;
    }
    else if (cmd == "KEY") {
        vm.variables[tokens[1]] = (GetAsyncKeyState(getValue(tokens[2], vm)) & 0x8000) ? 1 : 0;
    }
    else if (cmd == "goto") {
        if (vm.labels.count(tokens[1])) { vm.program_counter = vm.labels[tokens[1]]; return 2; }
    }
    else if (cmd == "if") {
        if (tokens.size() >= 6) {
            int v1 = getValue(tokens[1], vm);
            std::string op = tokens[2];
            int v2 = getValue(tokens[3], vm);
            bool res = (op == "==" && v1 == v2) || (op == "!=" && v1 != v2) || (op == ">" && v1 > v2) || (op == "<" && v1 < v2);
            if (res && tokens[4] == "goto") {
                vm.program_counter = vm.labels[tokens[5]]; return 2;
            }
        }
    }
    else if (cmd == "gosub") {
        vm.call_stack.push_back(vm.program_counter + 1);
        vm.program_counter = vm.labels[tokens[1]]; return 2;
    }
    else if (cmd == "return") {
        if (!vm.call_stack.empty()) { vm.program_counter = vm.call_stack.back(); vm.call_stack.pop_back(); return 2; }
    }
    else if (cmd == "SIN" || cmd == "COS") {
        float rad = getValue(tokens[2], vm) * 3.14159f / 180.0f;
        int res = (cmd == "SIN") ? (int)(sin(rad) * getValue(tokens[3], vm)) : (int)(cos(rad) * getValue(tokens[3], vm));
        setValue(tokens[1], res, vm);
    }
    else if (cmd == "exit") return 0;

    else {
        if (!cmd.empty()) {
            std::cout << "[Error]: Unknown command '" << cmd << "' at line " << vm.program_counter + 1 << std::endl;
            return 1;
        }
    }
    return 1;
}