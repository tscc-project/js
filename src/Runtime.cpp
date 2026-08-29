#include "Runtime.h"
#include "Version.h"
#include <new>

extern "C" {
const char*js_version(void){return JS_VERSION;}
js_runtime*js_runtime_new(void){try{return new js_runtime;}catch(...){return nullptr;}}
void js_runtime_free(js_runtime*r){if(!r)return;for(auto*v:r->values)delete v;delete r;}
const char*js_runtime_last_error(const js_runtime*r){return r?r->error.c_str():"invalid runtime";}
static js_status fail(js_runtime*r,js_status s,const char*m){if(r)r->error=m;return s;}
js_status js_eval(js_runtime*r,const char*source,js_value**result){
 if(result)*result=nullptr;
 if(!r||!source||!result)return fail(r,JS_STATUS_INVALID_ARGUMENT,"runtime, source and result are required");
 if(r->owner!=std::this_thread::get_id())return fail(r,JS_STATUS_WRONG_THREAD,"runtime used from a non-owner thread");
 return fail(r,JS_STATUS_UNSUPPORTED,"evaluation is not implemented until JS3");
}
void js_value_free(js_runtime*r,js_value*v){if(!r||!v)return;auto it=r->values.find(v);if(it!=r->values.end()){r->values.erase(it);delete v;}}
js_value_kind js_value_get_kind(const js_value*v){return v?v->kind:JS_VALUE_UNDEFINED;}
int js_value_get_boolean(const js_value*v,int*out){if(!v||!out||v->kind!=JS_VALUE_BOOLEAN)return 0;*out=v->boolean?1:0;return 1;}
int js_value_get_number(const js_value*v,double*out){if(!v||!out||v->kind!=JS_VALUE_NUMBER)return 0;*out=v->number;return 1;}
}
