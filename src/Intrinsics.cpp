#include "Intrinsics.h"

namespace jspp {
namespace {
js_value string_value(std::string text){js_value value;value.kind=JS_VALUE_STRING;value.string=std::move(text);return value;}
js_value number_value(double number){js_value value;value.kind=JS_VALUE_NUMBER;value.number=number;return value;}
}

js_value make_native(Heap&heap,const std::string&name,std::size_t length,bool constructible,
                     NativeFunction callback,const std::shared_ptr<ObjectValue>&function_prototype,
                     const std::shared_ptr<ObjectValue>&instance_prototype){
    js_value value;value.kind=JS_VALUE_FUNCTION;value.function=heap.function();
    value.function->native=std::move(callback);value.function->name=name;
    value.function->length=length;value.function->constructible=constructible;
    value.function->object_prototype=function_prototype;
    value.function->instance_prototype=instance_prototype;
    value.function->properties["name"]=string_value(name);
    value.function->properties["length"]=number_value(static_cast<double>(length));
    return value;
}

IntrinsicSet create_intrinsics(Heap&heap){
    IntrinsicSet set;set.global=heap.environment();set.object_prototype=heap.object();
    set.function_prototype=heap.object();set.function_prototype->prototype=set.object_prototype;
    auto object=make_native(heap,"Object",1,true,
        [prototype=set.object_prototype](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&,Heap&heap,const NativeInvoke&){
            if(!args.empty()&&(args[0].kind==JS_VALUE_OBJECT||args[0].kind==JS_VALUE_ARRAY||args[0].kind==JS_VALUE_FUNCTION)){out=args[0];return true;}
            out.kind=JS_VALUE_OBJECT;out.object=heap.object();out.object->prototype=prototype;return true;
        },set.function_prototype,set.object_prototype);
    set.global->bindings["Object"]={object,true};set.roots.push_back(object);
    return set;
}
}
