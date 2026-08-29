#pragma once
#include "Bytecode.h"
#include "Runtime.h"
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace jspp {
struct Binding { js_value value;bool constant=false; };
struct Environment { std::unordered_map<std::string,Binding>bindings;std::shared_ptr<Environment>parent; };
struct FunctionObject { std::shared_ptr<FunctionPrototype>prototype;std::shared_ptr<Environment>closure; };
bool execute(const Bytecode&,js_value&,std::string&error,std::size_t instruction_budget=1000000);
}
