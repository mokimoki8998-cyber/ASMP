#pragma once
#ifndef VM_H
#define VM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>

struct VM {
    std::map<std::string, int> variables;
    int program_counter = 0;
    std::map<std::string, int> labels;
    std::vector<bool> physical_ram = std::vector<bool>(8000000, false);
    std::vector<int> ram = std::vector<int>(10000, 0);
    std::vector<int> call_stack;
    HANDLE target_process_handle = NULL;
};

// токенизация, вроде должна показывать кирилицу
std::vector<std::string> tokenize(const std::string& line);

#endif