#include "VM.h"
namespace jspp {
std::shared_ptr<Environment>Heap::environment(){auto v=std::make_shared<Environment>();environments_.push_back(v);++allocations_;return v;}
std::shared_ptr<FunctionObject>Heap::function(){auto v=std::make_shared<FunctionObject>();functions_.push_back(v);++allocations_;return v;}
std::shared_ptr<ObjectValue>Heap::object(){auto v=std::make_shared<ObjectValue>();objects_.push_back(v);++allocations_;return v;}
std::shared_ptr<ArrayValue>Heap::array(){auto v=std::make_shared<ArrayValue>();arrays_.push_back(v);++allocations_;return v;}
std::size_t Heap::tracked()const{std::size_t n=0;for(const auto&v:environments_)n+=!v.expired();for(const auto&v:functions_)n+=!v.expired();for(const auto&v:objects_)n+=!v.expired();for(const auto&v:arrays_)n+=!v.expired();return n;}
}
