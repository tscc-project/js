#include "Bytecode.h"
#include "Frontend.h"
#include "js.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
static void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
int main(){jspp::Program p;jspp::Diagnostic d;check(jspp::parse_source("const x=2+3*4;x>=14;",p,d),"parse");jspp::Bytecode b;std::string e;check(jspp::compile(p,b,e),"compile");auto listing=jspp::disassemble(b);check(listing.find("multiply")!=std::string::npos&&listing.find("store x")!=std::string::npos,"bytecode inspection");auto*r=js_runtime_new();js_value*v=nullptr;check(js_eval(r,"let x=6; (x*7)===42;",&v)==JS_STATUS_OK,"execute");int yes=0;check(js_value_get_boolean(v,&yes)&&yes,"boolean result");js_value_free(r,v);check(js_eval(r,"'JS'+'++';",&v)==JS_STATUS_OK,"string execute");const char*s=nullptr;size_t n=0;check(js_value_get_string(v,&s,&n)&&std::string(s,n)=="JS++","string result");js_value_free(r,v);check(js_eval(r,"missing+1",&v)==JS_STATUS_RUNTIME_ERROR,"missing identifier accepted");check(js_eval(r,"1+true",&v)==JS_STATUS_RUNTIME_ERROR,"mixed arithmetic accepted");check(js_eval(r,"let x=1;let x=2;",&v)==JS_STATUS_SYNTAX_ERROR,"duplicate binding accepted");js_runtime_free(r);std::cout<<"JS++ primitive VM passed\n";}
