#pragma once
#include "Bytecode.h"
#include "Runtime.h"
namespace jspp { bool execute(const Bytecode&,js_value&,std::string&error); }
