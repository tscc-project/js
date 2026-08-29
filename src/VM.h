#pragma once
#include "Bytecode.h"
#include "Runtime.h"
#include <cstddef>
namespace jspp { bool execute(const Bytecode&,js_value&,std::string&error,std::size_t instruction_budget=1000000); }
