#pragma once
#include "js.h"
#include <string>
#include <thread>
#include <unordered_set>

struct js_value { js_value_kind kind=JS_VALUE_UNDEFINED; bool boolean=false; double number=0; };
struct js_runtime {
 std::thread::id owner=std::this_thread::get_id();
 std::string error;
 std::unordered_set<js_value*> values;
};
