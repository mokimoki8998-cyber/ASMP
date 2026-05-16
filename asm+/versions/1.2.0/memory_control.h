#pragma once
#ifndef MEMORY_CONTROL_H
#define MEMORY_CONTROL_H

#include "vm.h"

int getValue(const std::string& token, VM& vm);
void setValue(const std::string& target, int value, VM& vm);
bool isMemoryAccess(const std::string& token);
void dumpMemory(VM& vm, int start, int length);

#endif