#include "Bytecode.h"
#include "Frontend.h"
#include "VM.h"
#include "Property.h"
#include "js.h"
#include <cstdlib>
#include <iostream>
#include <string>
static void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
static double number(js_runtime*r,const char*source){js_value*v=nullptr;check(js_eval(r,source,&v)==JS_STATUS_OK,"numeric evaluation");double n=0;check(js_value_get_number(v,&n),"numeric result");js_value_free(r,v);return n;}
int main(){
 jspp::Heap property_heap;js_value property_object;property_object.kind=JS_VALUE_OBJECT;property_object.object=property_heap.object();js_value property_value;property_value.kind=JS_VALUE_NUMBER;property_value.number=42;jspp::PropertyAttributes property_flags;property_flags.writable=false;jspp::define_own_property(jspp::own_property_maps(property_object),"answer",property_value,property_flags);js_value read_property;jspp::PropertyAttributes read_flags;check(jspp::own_property(jspp::own_property_maps(property_object),"answer",read_property,read_flags)&&read_property.number==42&&!read_flags.writable,"central property operations");check(jspp::own_property_keys(jspp::own_property_maps(property_object),true)==std::vector<std::string>{"answer"},"central own-key operation");
 jspp::Program thrown_program;jspp::Diagnostic thrown_diagnostic;check(jspp::parse_source("throw 42;",thrown_program,thrown_diagnostic),"completion parse");jspp::Bytecode thrown_code;std::string thrown_error;check(jspp::compile(thrown_program,thrown_code,thrown_error),"completion compile");jspp::Completion completion;check(!jspp::execute_completion(thrown_code,completion,thrown_error)&&completion.kind==jspp::CompletionKind::Throw&&completion.value.kind==JS_VALUE_NUMBER&&completion.value.number==42,"explicit throw completion");
 jspp::Bytecode handled;handled.instructions={{jspp::Op::PushNumber,42},{jspp::Op::Throw},{jspp::Op::Pop},{jspp::Op::PushNumber,7}};handled.handlers.push_back({0,2,2,0,0});jspp::Completion caught;std::string handler_error;check(jspp::execute_completion(handled,caught,handler_error)&&caught.kind==jspp::CompletionKind::Normal&&caught.value.number==7,"handler region did not resume");jspp::Bytecode malformed=handled;malformed.handlers[0].target=99;jspp::Completion rejected;handler_error.clear();check(!jspp::execute_completion(malformed,rejected,handler_error)&&handler_error=="invalid exception handler bytecode","malformed handler accepted");
 jspp::Program cross_program;check(jspp::parse_source("function fail(){throw 42;}fail();",cross_program,thrown_diagnostic),"cross-frame handler parse");jspp::Bytecode cross;check(jspp::compile(cross_program,cross,handler_error),"cross-frame handler compile");const auto handler_target=cross.instructions.size();cross.handlers.push_back({0,handler_target,handler_target,0,0});cross.instructions.push_back({jspp::Op::Pop});cross.instructions.push_back({jspp::Op::PushNumber,9});jspp::Completion cross_result;handler_error.clear();check(jspp::execute_completion(cross,cross_result,handler_error)&&cross_result.value.number==9,"cross-frame throw did not reach handler");
 jspp::Bytecode cleanup;cleanup.instructions={{jspp::Op::PushNumber,42},{jspp::Op::Throw},{jspp::Op::PushUndefined},{jspp::Op::ResumeCompletion}};cleanup.cleanups.push_back({0,2,2,0,0});jspp::Completion cleaned;std::string cleanup_error;check(!jspp::execute_completion(cleanup,cleaned,cleanup_error)&&cleaned.kind==jspp::CompletionKind::Throw&&cleaned.value.number==42,"cleanup lost pending throw");jspp::Bytecode replaced=cleanup;replaced.instructions[2]={jspp::Op::PushNumber,9};replaced.instructions[3]={jspp::Op::Return};cleanup_error.clear();check(!jspp::execute_completion(replaced,cleaned,cleanup_error)&&cleanup_error=="return escaped top-level execution"&&cleaned.value.number==9,"cleanup return did not replace throw");jspp::Bytecode orphan;orphan.instructions={{jspp::Op::ResumeCompletion}};cleanup_error.clear();check(!jspp::execute_completion(orphan,cleaned,cleanup_error)&&cleanup_error=="invalid pending completion bytecode","orphan completion resume accepted");
 jspp::Program p;jspp::Diagnostic d;check(jspp::parse_source("const x=2+3*4;x>=14;",p,d),"parse");jspp::Bytecode b;std::string e;check(jspp::compile(p,b,e),"compile");auto listing=jspp::disassemble(b);check(listing.find("multiply")!=std::string::npos&&listing.find("declare x")!=std::string::npos,"bytecode inspection");
 auto*r=js_runtime_new();js_value*v=nullptr;check(js_eval(r,"let x=6;(x*7)===42;",&v)==JS_STATUS_OK,"execute");int yes=0;check(js_value_get_boolean(v,&yes)&&yes,"boolean result");js_value_free(r,v);
 check(number(r,"let sum=0;for(let i=0;i<8;i=i+1){if(i===2)continue;if(i===6)break;sum=sum+i;}sum;")==13,"control flow result");
 check(number(r,"function outer(x){return function(y){x=x+1;return x+y;};}const f=outer(39);f(1);f(1);")==42,"closure capture result");
 check(number(r,"function fact(n){if(n<=1)return 1;return n*fact(n-1);}fact(6);")==720,"recursive result");
 check(number(r,"const add=(a,b)=>a+b;add(20,22);")==42,"arrow result");
 check(number(r,"const o={answer:40};o.answer=o.answer+2;o['answer'];")==42,"object property result");
 check(number(r,"const a=[10,20];a[2]=12;a[0]+a[1]+a[2];")==42,"array property result");
 check(number(r,"const a={x:1};const b=a;b.x=42;a.x;")==42,"object aliasing result");
 check(number(r,"const x=42;({x}).x;")==42,"object shorthand result");
 check(number(r,"[1,,3].length;")==3,"array hole length result");
 check(number(r,"const o={x:40,add(n){this.x=this.x+n;return this.x;}};o.add(2);")==42,"method receiver result");
 check(number(r,"function Answer(n){this.value=n;}Answer.prototype.read=function(){return this.value;};const a=new Answer(42);a.read();")==42,"constructor prototype result");
 check(number(r,"let x=1;{let x=2;}x;")==1,"block scope result");
 js_value*thrown=nullptr;check(js_eval(r,"function fail(){throw 42;}fail();",&thrown)==JS_STATUS_RUNTIME_ERROR,"throw did not propagate");check(std::string(js_runtime_last_error(r))=="uncaught exception: 42","throw diagnostic");
 check(number(r,"try{throw 41;}catch(error){error+1;}")==42,"catch binding result");
 check(number(r,"function fail(){throw 40;}try{fail();}catch(error){error+2;}")==42,"cross-frame catch result");
 check(number(r,"try{try{throw 40;}catch(inner){throw inner+1;}}catch(outer){outer+1;}")==42,"nearest nested catch result");
 check(number(r,"function answer(){try{throw 1;}catch(error){return error+41;}}answer();")==42,"return from catch result");
 check(number(r,"try{throw 1;}catch{40+2;}")==42,"optional catch binding result");
 check(number(r,"1+true;")==2,"boolean numeric conversion");check(number(r,"'42'-2;")==40,"string numeric conversion");check(number(r,"const a=[42];a['0'];")==42,"string array index conversion");check(number(r,"const o={};o[true]=42;o['true'];")==42,"property key conversion");check(js_eval(r,"const fixed=1;fixed=2;",&v)==JS_STATUS_RUNTIME_ERROR,"const assignment accepted");check(js_eval(r,"missing+1",&v)==JS_STATUS_RUNTIME_ERROR,"missing identifier accepted");check(js_eval(r,"let x=1;let x=2;",&v)==JS_STATUS_SYNTAX_ERROR,"duplicate binding accepted");
 js_runtime_free(r);std::cout<<"JS++ functions/closures VM passed\n";
}
