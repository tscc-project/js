#include "js.h"
#include <cstring>
#include <iostream>
#include <thread>
static void require(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
int main(){
 require(std::strlen(js_version())>0,"missing version");js_runtime_free(nullptr);js_value_free(nullptr,nullptr);
 for(int i=0;i<1000;++i){auto*r=js_runtime_new();require(r,"runtime allocation failed");js_value*v=nullptr;require(js_eval(r,"1",&v)==JS_STATUS_OK,"evaluation failed");require(v!=nullptr,"successful eval omitted value");js_value_free(r,v);js_runtime_free(r);}
 auto*r=js_runtime_new();js_status status=JS_STATUS_OK;std::thread t([&]{js_value*v=nullptr;status=js_eval(r,"1",&v);});t.join();require(status==JS_STATUS_WRONG_THREAD,"thread affinity not enforced");js_runtime_free(r);
 std::cout<<"JS++ lifecycle passed\n";
}
