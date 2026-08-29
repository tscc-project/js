#include "Runtime.h"
#include "Bytecode.h"
#include "Frontend.h"
#include "VM.h"
#include "Version.h"
#include <new>
#include <memory>

extern "C" {
const char*js_version(void){return JS_VERSION;}
js_runtime*js_runtime_new(void){try{auto*r=new js_runtime;r->heap=std::make_shared<jspp::Heap>();return r;}catch(...){return nullptr;}}
static void collect(js_runtime*r){std::vector<js_value>roots;for(auto*v:r->values)roots.push_back(*v);r->heap->collect(roots);}
void js_runtime_free(js_runtime*r){if(!r)return;for(auto*v:r->values)delete v;r->values.clear();collect(r);delete r;}
const char*js_runtime_last_error(const js_runtime*r){return r?r->error.c_str():"invalid runtime";}
static js_status fail(js_runtime*r,js_status s,const char*m){if(r)r->error=m;return s;}
js_status js_eval(js_runtime*r,const char*source,js_value**result){
 if(result)*result=nullptr;
 if(!r||!source||!result)return fail(r,JS_STATUS_INVALID_ARGUMENT,"runtime, source and result are required");
 if(r->owner!=std::this_thread::get_id())return fail(r,JS_STATUS_WRONG_THREAD,"runtime used from a non-owner thread");
 try{jspp::Program program;jspp::Diagnostic diagnostic;if(!jspp::parse_source(source,program,diagnostic))return fail(r,JS_STATUS_SYNTAX_ERROR,jspp::format_diagnostic(diagnostic).c_str());jspp::Bytecode code;std::string error;if(!jspp::compile(program,code,error))return fail(r,JS_STATUS_SYNTAX_ERROR,error.c_str());auto value=std::make_unique<js_value>();if(!jspp::execute(code,*value,error,1000000,r->heap.get()))return fail(r,JS_STATUS_RUNTIME_ERROR,error.c_str());r->values.insert(value.get());collect(r);r->error.clear();*result=value.release();return JS_STATUS_OK;}catch(const std::bad_alloc&){return fail(r,JS_STATUS_OUT_OF_MEMORY,"out of memory");}catch(...){return fail(r,JS_STATUS_RUNTIME_ERROR,"internal runtime failure");}
}
void js_value_free(js_runtime*r,js_value*v){if(!r||!v)return;auto it=r->values.find(v);if(it!=r->values.end()){r->values.erase(it);delete v;collect(r);}}
js_value_kind js_value_get_kind(const js_value*v){return v?v->kind:JS_VALUE_UNDEFINED;}
int js_value_get_boolean(const js_value*v,int*out){if(!v||!out||v->kind!=JS_VALUE_BOOLEAN)return 0;*out=v->boolean?1:0;return 1;}
int js_value_get_number(const js_value*v,double*out){if(!v||!out||v->kind!=JS_VALUE_NUMBER)return 0;*out=v->number;return 1;}
int js_value_get_string(const js_value*v,const char**data,size_t*size){if(!v||!data||!size||v->kind!=JS_VALUE_STRING)return 0;*data=v->string.data();*size=v->string.size();return 1;}
}
