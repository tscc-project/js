#pragma once
#include "Frontend.h"
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
namespace jspp {
enum class Op { PushUndefined,PushNull,PushBoolean,PushNumber,PushString,Load,Declare,Store,UnaryPlus,Negate,Not,Add,Subtract,Multiply,Divide,Remainder,Equal,NotEqual,StrictEqual,StrictNotEqual,Less,LessEqual,Greater,GreaterEqual,Pop,Duplicate,Jump,JumpIfFalse,JumpIfTrue };
struct Instruction { Op op;double number;bool boolean;std::string text;std::size_t target=0;Instruction(Op o=Op::PushUndefined,double n=0,bool b=false,std::string t={},std::size_t j=0):op(o),number(n),boolean(b),text(std::move(t)),target(j){} };
struct Bytecode { std::vector<Instruction> instructions; };
bool compile(const Program&,Bytecode&,std::string&error);
std::string disassemble(const Bytecode&);
}
