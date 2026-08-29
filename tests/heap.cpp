#include "Bytecode.h"
#include "Frontend.h"
#include "VM.h"
#include <cstdlib>
#include <iostream>
static void check(bool value,const char*message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main(){jspp::Program program;jspp::Diagnostic diagnostic;jspp::Bytecode code;std::string error;check(jspp::parse_source("function Box(x){this.x=x;}const box=new Box(42);box;",program,diagnostic),"heap fixture parse");check(jspp::compile(program,code,error),"heap fixture compile");jspp::Heap heap;js_value result;check(jspp::execute(code,result,error,1000000,&heap),"heap fixture execute");check(heap.allocations()>=5,"heap did not own allocations");check(heap.tracked()>=2,"reachable graph not tracked");std::cout<<"JS++ traced heap foundation passed\n";}
