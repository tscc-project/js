#include "Bytecode.h"
#include "Frontend.h"
#include "VM.h"
#include "js.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

static void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
static jspp::Bytecode compile(const char*source){jspp::Program p;jspp::Diagnostic d;check(jspp::parse_source(source,p,d),"fixture parse");jspp::Bytecode b;std::string e;check(jspp::compile(p,b,e),"fixture compile");return b;}

int main(){
 /* Reduced reproduction: declared function + throw, primitive completion. The
    top-level environment <-> declared-function cycle must be reclaimed by the
    local-heap final sweep, not retained. */
 {auto b=compile("function fail(){throw 42;}fail();");jspp::Completion c;std::string e;check(!jspp::execute_completion(b,c,e),"declared-throw accepted");check(c.kind==jspp::CompletionKind::Throw&&c.value.kind==JS_VALUE_NUMBER&&c.value.number==42,"declared-throw completion");check(e=="uncaught exception: 42","declared-throw diagnostic");}
 /* Arrow closure declared but call completes a primitive. */
 {auto b=compile("const f=()=>42;f();");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"arrow call failed");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_NUMBER&&c.value.number==42,"arrow call completion");}
 /* Repeated below-threshold standalone executions must not accumulate retained
    cycles: every iteration uses its own local heap and final sweep. */
 {for(int i=0;i<1000;++i){auto b=compile("const o={};o.self=o;1;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"repeated self-cycle execution");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_NUMBER&&c.value.number==1,"repeated self-cycle completion");}}
 /* Every exit class cleans up: normal, throw, malformed bytecode, instruction
    budget, stack limit and escaped top-level return each finish through the
    local-heap sweep (and teardown collection where the value pins the heap). */
 {auto good=compile("const a=1;a;");jspp::Completion c;std::string e;check(jspp::execute_completion(good,c,e),"success path failed");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_NUMBER&&c.value.number==1,"success path completion");}
 {auto bad=compile("throw 7;");jspp::Completion c;std::string e;check(!jspp::execute_completion(bad,c,e),"throw path accepted");check(c.kind==jspp::CompletionKind::Throw&&c.value.kind==JS_VALUE_NUMBER&&c.value.number==7,"throw path completion");}
 {jspp::Bytecode malformed;malformed.instructions={{jspp::Op::PushNumber,42},{jspp::Op::Throw},{jspp::Op::Pop},{jspp::Op::PushNumber,7}};malformed.handlers.push_back({0,2,2,99,0});jspp::Completion c;std::string e;check(!jspp::execute_completion(malformed,c,e),"malformed handler accepted");check(e=="invalid exception handler bytecode","malformed handler diagnostic");}
 {auto loop=compile("while(true){}");jspp::Completion c;std::string e;check(!jspp::execute_completion(loop,c,e,10),"infinite loop accepted");check(e=="execution limit exceeded","instruction-budget diagnostic");}
 {auto recurse=compile("function r(){r();}r();");jspp::Completion c;std::string e;check(!jspp::execute_completion(recurse,c,e,1000000,nullptr,{},2),"stack-limit accepted");check(e=="call stack limit exceeded","stack-limit diagnostic");}
 {jspp::Bytecode escape;escape.instructions={{jspp::Op::PushNumber,5},{jspp::Op::Return}};jspp::Completion c;std::string e;check(!jspp::execute_completion(escape,c,e),"escaped return accepted");check(e=="return escaped top-level execution","escaped return diagnostic");}
 /* Supplied-heap execution retains reachable results until the caller releases
    them and invokes collection; no exit-time sweep touches a caller heap. */
 {jspp::Heap heap;auto b=compile("function Box(x){this.x=x;}const box=new Box(42);box;");js_value result;std::string e;check(jspp::execute(b,result,e,1000000,&heap),"supplied-heap execute");check(result.kind==JS_VALUE_OBJECT&&result.object&&result.object->properties.count("x"),"supplied-heap result live");check(result.object->properties["x"].number==42,"supplied-heap result value");check(heap.tracked()>=2,"supplied-heap graph not tracked");std::size_t reclaimed_rooted=heap.collect({result});check(result.object&&result.object->properties.count("x"),"rooted result reclaimed by caller collect");check(reclaimed_rooted>=1,"unrooted cycle not reclaimed while result rooted");result={};heap.collect({});check(heap.tracked()==0,"released graph retained after caller collection");}
 /* Runtime embedding behaviour is unchanged: the runtime owns its heap and
    global, collects after evaluation, and returns valid results. */
 {auto*r=js_runtime_new();check(r,"runtime");js_value*v=nullptr;check(js_eval(r,"function f(){return 42;}f();",&v)==JS_STATUS_OK,"runtime eval");double n=0;check(js_value_get_number(v,&n)&&n==42,"runtime result");js_value_free(r,v);js_runtime_free(r);}
 /* Local-heap contract: object, array and function completion values are
    transferred to the caller as owning shared_ptrs and remain valid after the
    call. This includes self-referential and mutually reachable graphs, which
    pin the local heap and are reclaimed when the value is released. */
 {auto b=compile("const o={answer:42};o;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"object completion execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_OBJECT&&c.value.object&&c.value.object->properties.count("answer"),"object completion value");check(c.value.object->properties["answer"].number==42,"object completion contents");}
 {auto b=compile("[1,2,3];");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"array completion execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_ARRAY&&c.value.array&&c.value.array->elements.size()==3,"array completion value");}
 {auto b=compile("(function(){return function inner(){return 42;};})();");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"function completion execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_FUNCTION&&c.value.function,"function completion value");}
 {auto b=compile("const o={};o.self=o;o;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"self-referential object execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_OBJECT&&c.value.object&&c.value.object->properties.count("self")&&c.value.object->properties["self"].kind==JS_VALUE_OBJECT&&c.value.object->properties["self"].object==c.value.object,"self-referential object completion");}
 {auto b=compile("const a=[];a[0]=a;a;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"self-referential array execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_ARRAY&&c.value.array&&c.value.array->elements.size()>=1&&c.value.array->elements[0].kind==JS_VALUE_ARRAY&&c.value.array->elements[0].array==c.value.array,"self-referential array completion");}
 {auto b=compile("const f=()=>42; f;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"self-referential closure execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_FUNCTION&&c.value.function,"self-referential closure completion");}
 {auto b=compile("const a={};const b={};a.b=b;b.a=a;a;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e),"mutually reachable execute");check(c.kind==jspp::CompletionKind::Normal&&c.value.kind==JS_VALUE_OBJECT&&c.value.object&&c.value.object->properties.count("b")&&c.value.object->properties["b"].kind==JS_VALUE_OBJECT&&c.value.object->properties["b"].object->properties.count("a"),"mutually reachable completion");}
 /* Caller heap + caller global: a self-referential value written into the
    persistent global during execution stays intact, is retained while the
    caller roots the global, and is reclaimed when the caller releases the
    binding and collects. */
 {jspp::Heap heap;auto global=std::make_shared<jspp::Environment>();global->bindings["g"]={js_value{},false};auto b=compile("g={};g.self=g;1;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e,1000000,&heap,global),"caller global execute");auto&g=global->bindings["g"].value;check(g.kind==JS_VALUE_OBJECT&&g.object&&g.object->properties.count("self")&&g.object->properties["self"].object==g.object,"global cycle written");check(heap.collect({}, {global})==0,"rooted global reclaimed");check(g.object&&g.object->properties.count("self"),"rooted global value survived collect");global->bindings.clear();check(heap.collect({})>=1,"released global cycle not reclaimed");check(heap.tracked()==0,"caller heap retains released global graph");}
 /* Caller global without caller heap is rejected deterministically before
    execution; no combination may appear supported while losing tracking. */
 {auto global=std::make_shared<jspp::Environment>();auto b=compile("1;");jspp::Completion c;std::string e;check(!jspp::execute_completion(b,c,e,1000000,nullptr,global),"caller global without heap accepted");check(e=="caller-owned heap is required when a caller-owned global is supplied","rejection diagnostic");}
 /* Caller heap without caller global allocates a fresh top-level environment. */
 {jspp::Heap heap;auto b=compile("2+2;");jspp::Completion c;std::string e;check(jspp::execute_completion(b,c,e,1000000,&heap),"heap without global failed");check(c.value.kind==JS_VALUE_NUMBER&&c.value.number==4,"heap without global result");}
 std::cout<<"JS++ standalone execution lifetime passed\n";
}