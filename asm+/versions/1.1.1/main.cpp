#include "vm.h"
#include "commands.h"
#include "memory_control.h"
#include <fstream>
#include <sstream>  

// 22:25 15.05.26: я случайно сломал экран телефона пока програмировал
//если ты это читаешь, у меня уже есть заготовленые команды, я их добавлю на обнову 1.2.0, а щяс обнова 1.1.1 (модулезация и исправление команды run)

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string t;
    while (ss >> t) tokens.push_back(t);
    return tokens;
}

void runScript(const std::string& filename, VM& vm) {
    std::ifstream f(filename);
    std::vector<std::string> lines;
    std::string l;
    while (std::getline(f, l)) lines.push_back(l);

    for (int i = 0; i < (int)lines.size(); i++) {
        std::vector<std::string> t = tokenize(lines[i]);
        if (!t.empty() && t[0][0] == ':') vm.labels[t[0]] = i;
    }

    vm.program_counter = 0;
    while (vm.program_counter < (int)lines.size()) {
        int res = executeLine(lines[vm.program_counter], vm);
        if (res == 0) break;
        if (res == 1) vm.program_counter++;
    }
}

int main(int argc, char* argv[]) {
    system("chcp 65001 > nul");
    VM vm;
    if (argc >= 2) {
        runScript(argv[1], vm);
        return 0;
    }
    std::cout << "=== ASM+ 1.0.0 (Modular) ===" << std::endl;
    std::string input;
    while (std::cout << "asm+ > " && std::getline(std::cin, input)) {
        if (input == "exit") break;
        executeLine(input, vm);
    }
    return 0;
}