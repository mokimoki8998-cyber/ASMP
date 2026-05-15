#include "memory_control.h"
#include <sstream> // "stringstream"
#include <iomanip> // "setw/setfill"
#include <string> // "operator >>"

bool isMemoryAccess(const std::string& token) {
    return token.length() >= 3 && token.front() == '[' && token.back() == ']';
}

int getValue(const std::string& token, VM& vm) {
    if (isMemoryAccess(token)) {
        std::string inside = token.substr(1, token.length() - 2);
        int address = getValue(inside, vm);
        if (address >= 0 && address < (int)vm.ram.size()) return vm.ram[address];
        return 0;
    }
    try {
        size_t idx;
        int val = std::stoi(token, &idx);
        if (idx == token.length()) return val;
    }
    catch (...) {}
    return vm.variables[token];
}

void setValue(const std::string& target, int value, VM& vm) {
    if (isMemoryAccess(target)) {
        std::string inside = target.substr(1, target.length() - 2);
        int address = getValue(inside, vm);
        if (address >= 0 && address < (int)vm.ram.size()) vm.ram[address] = value;
    }
    else {
        try { std::stoi(target); return; }
        catch (...) {}
        vm.variables[target] = value;
    }
}

void dumpMemory(VM& vm, int start, int length) {
    std::cout << "\n--- [PHYSICAL RAM BINARY TABLE] ---\nADDR | 0 1 2 3 4 5 6 7 | HEX\n";
    for (int i = start; i < start + length; i += 8) {
        if (i >= (int)vm.physical_ram.size()) break;
        std::cout << std::setw(4) << std::setfill('0') << i << " | ";
        int byte_val = 0;
        for (int j = 0; j < 8; ++j) {
            if (i + j < (int)vm.physical_ram.size()) {
                bool bit = vm.physical_ram[i + j];
                std::cout << (bit ? "1 " : "0 ");
                if (bit) byte_val |= (1 << (7 - j));
            }
        }
        std::cout << "| 0x" << std::hex << byte_val << std::dec << "\n";
    }
}