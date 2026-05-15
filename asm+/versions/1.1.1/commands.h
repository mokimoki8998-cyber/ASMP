#pragma once
#ifndef COMMANDS_H
#define COMMANDS_H

#include "vm.h"

// без этой хуйни ничего работать не будет
int executeLine(const std::string& current_line, VM& vm);

#endif

void runScript(const std::string& filename, VM& vm);