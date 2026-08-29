#pragma once
#include "Frontend.h"
#include <string>
#include <vector>
#include <utility>
namespace jspp {
enum class Op { PushUndefined,PushNull,PushBoolean,PushNumber,PushString,Load,Store,UnaryPlus,Negate,Not,Add,Subtract,Multiply,Divide,Remainder,Equal,NotEqual,StrictEqual,StrictNotEqual,Less,LessEqual,Greater,GreaterEqual,Pop };
struct Instruction { Op op;double number;bool boolean;std::string text;Instruction(Op o=Op::PushUndefined,double n=0,bool b=false,std::string t={}):op(o),number(n),boolean(b),text(std::move(t)){} };
struct Bytecode { std::vector<Instruction> instructions; };
bool compile(const Program&,Bytecode&,std::string&error);
std::string disassemble(const Bytecode&);
}
