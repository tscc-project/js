#include "js.h"
#include <cstring>
#include <iostream>
#include <thread>
static void require(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
int main(){
 require(std::strlen(js_version())>0,"missing version");js_runtime_free(nullptr);js_value_free(nullptr,nullptr);
 for(int i=0;i<1000;++i){auto*r=js_runtime_new();require(r,"runtime allocation failed");js_value*v=nullptr;require(js_eval(r,"1",&v)==JS_STATUS_OK,"evaluation failed");require(v!=nullptr,"successful eval omitted value");js_value_free(r,v);js_runtime_free(r);}
 auto*gc=js_runtime_new();for(int i=0;i<200;++i){js_value*v=nullptr;require(js_eval(gc,"const o={};o.self=o;o;",&v)==JS_STATUS_OK,"cycle evaluation failed");require(js_value_get_kind(v)==JS_VALUE_OBJECT,"cycle result lost");js_value_free(gc,v);}js_value*held=nullptr;require(js_eval(gc,"({answer:42});",&held)==JS_STATUS_OK,"root fixture failed");js_value*pressure=nullptr;require(js_eval(gc,"const o={};o.self=o;1;",&pressure)==JS_STATUS_OK,"root pressure failed");require(js_value_get_kind(held)==JS_VALUE_OBJECT,"persistent handle was not rooted");js_value_free(gc,pressure);js_value_free(gc,held);js_runtime_free(gc);
 auto*r=js_runtime_new();js_status status=JS_STATUS_OK;std::thread t([&]{js_value*v=nullptr;status=js_eval(r,"1",&v);});t.join();require(status==JS_STATUS_WRONG_THREAD,"thread affinity not enforced");js_runtime_free(r);
 auto*intrinsics=js_runtime_new();js_value*first=nullptr;require(js_eval(intrinsics,"Object.marker=42;Object.name;",&first)==JS_STATUS_OK,"intrinsic setup failed");const char*name=nullptr;size_t name_size=0;require(js_value_get_string(first,&name,&name_size)&&std::string(name,name_size)=="Object","native function identity missing");js_value_free(intrinsics,first);js_value*second=nullptr;require(js_eval(intrinsics,"Object.marker;",&second)==JS_STATUS_OK,"intrinsic registry did not persist");double marker=0;require(js_value_get_number(second,&marker)&&marker==42,"intrinsic identity changed between evaluations");js_value_free(intrinsics,second);js_runtime_free(intrinsics);
 std::cout<<"JS++ lifecycle passed\n";
}
