#pragma once
#include "js.h"
#include <string>
#include <memory>
#include <thread>
#include <unordered_set>

namespace jspp { struct FunctionObject; }
struct js_value { js_value_kind kind=JS_VALUE_UNDEFINED; bool boolean=false; double number=0; std::string string; std::shared_ptr<jspp::FunctionObject> function; };
struct js_runtime {
 std::thread::id owner=std::this_thread::get_id();
 std::string error;
 std::unordered_set<js_value*> values;
};
