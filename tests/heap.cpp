#include "Bytecode.h"
#include "Frontend.h"
#include "VM.h"
#include <cstdlib>
#include <iostream>
static void check(bool value,const char*message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main(){jspp::Program program;jspp::Diagnostic diagnostic;jspp::Bytecode code;std::string error;check(jspp::parse_source("function Box(x){this.x=x;}const box=new Box(42);box;",program,diagnostic),"heap fixture parse");check(jspp::compile(program,code,error),"heap fixture compile");jspp::Heap heap;js_value result;check(jspp::execute(code,result,error,1000000,&heap),"heap fixture execute");check(heap.allocations()>=5,"heap did not own allocations");check(heap.tracked()>=2,"reachable graph not tracked");heap.collect({result});check(result.object&&result.object->properties.count("x"),"rooted result was reclaimed");result={};heap.collect({});
 jspp::Heap cycles;auto environment=cycles.environment();auto function=cycles.function();function->closure=environment;js_value closure;closure.kind=JS_VALUE_FUNCTION;closure.function=function;environment->bindings["self"]={closure,true};std::weak_ptr<jspp::FunctionObject>weak_function=function;std::weak_ptr<jspp::Environment>weak_environment=environment;closure={};function.reset();environment.reset();check(!weak_function.expired()&&!weak_environment.expired(),"cycle fixture did not cycle");check(cycles.collect({})>=2,"cycle was not selected");check(weak_function.expired()&&weak_environment.expired(),"cycle was not reclaimed");std::cout<<"JS++ traced heap and cycle collection passed\n";}
