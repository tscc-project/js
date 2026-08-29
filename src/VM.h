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
struct FunctionObject { std::shared_ptr<FunctionPrototype>prototype;std::shared_ptr<Environment>closure;std::unordered_map<std::string,js_value>properties;std::shared_ptr<ObjectValue>instance_prototype; };
struct ObjectValue { std::unordered_map<std::string,js_value>properties;std::shared_ptr<ObjectValue>prototype; };
struct ArrayValue { std::vector<js_value>elements;std::unordered_map<std::string,js_value>properties;std::shared_ptr<ObjectValue>prototype; };
class Heap {
public:
 std::shared_ptr<Environment>environment();std::shared_ptr<FunctionObject>function();
 std::shared_ptr<ObjectValue>object();std::shared_ptr<ArrayValue>array();
 std::size_t allocations()const{return allocations_;}std::size_t tracked()const;
 std::size_t collect(const std::vector<js_value>&roots,const std::vector<std::shared_ptr<Environment>>&environment_roots={});
 bool collection_due()const{return allocations_>=next_collection_;}
 std::size_t collections()const{return collections_;}
private:
 std::size_t allocations_=0,next_collection_=64,collections_=0;bool collecting_=false;std::vector<std::weak_ptr<Environment>>environments_;
 std::vector<std::weak_ptr<FunctionObject>>functions_;std::vector<std::weak_ptr<ObjectValue>>objects_;
 std::vector<std::weak_ptr<ArrayValue>>arrays_;
};
bool execute(const Bytecode&,js_value&,std::string&error,std::size_t instruction_budget=1000000,Heap*heap=nullptr);
}
