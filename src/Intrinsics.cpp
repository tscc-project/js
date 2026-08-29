#include "Intrinsics.h"
#include "Conversion.h"

namespace jspp {
namespace {
js_value string_value(std::string text){js_value value;value.kind=JS_VALUE_STRING;value.string=std::move(text);return value;}
js_value number_value(double number){js_value value;value.kind=JS_VALUE_NUMBER;value.number=number;return value;}
js_value undefined_value(){return {};}
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
    auto value_of=make_native(heap,"valueOf",0,false,
        [](const std::vector<js_value>&,const js_value&receiver,bool,js_value&out,std::string&,Heap&,const NativeInvoke&){out=receiver;return true;},
        set.function_prototype,set.object_prototype);
    auto object_to_string=make_native(heap,"toString",0,false,
        [](const std::vector<js_value>&,const js_value&,bool,js_value&out,std::string&,Heap&,const NativeInvoke&){out=string_value("[object Object]");return true;},
        set.function_prototype,set.object_prototype);
    set.object_prototype->properties["valueOf"]=value_of;
    set.object_prototype->properties["toString"]=object_to_string;

    auto function_call=make_native(heap,"call",1,false,
        [](const std::vector<js_value>&args,const js_value&receiver,bool,js_value&out,std::string&error,Heap&,const NativeInvoke&invoke){
            if(receiver.kind!=JS_VALUE_FUNCTION){error="Function.prototype.call receiver is not callable";return false;}
            const auto this_value=args.empty()?undefined_value():args[0];
            std::vector<js_value>forwarded;if(args.size()>1)forwarded.assign(args.begin()+1,args.end());
            return invoke(receiver,forwarded,this_value,false,out);
        },set.function_prototype,set.object_prototype);
    auto function_apply=make_native(heap,"apply",2,false,
        [](const std::vector<js_value>&args,const js_value&receiver,bool,js_value&out,std::string&error,Heap&,const NativeInvoke&invoke){
            if(receiver.kind!=JS_VALUE_FUNCTION){error="Function.prototype.apply receiver is not callable";return false;}
            const auto this_value=args.empty()?undefined_value():args[0];std::vector<js_value>forwarded;
            if(args.size()>1&&args[1].kind!=JS_VALUE_NULL&&args[1].kind!=JS_VALUE_UNDEFINED){if(args[1].kind!=JS_VALUE_ARRAY||!args[1].array){error="Function.prototype.apply arguments must be an array";return false;}forwarded=args[1].array->elements;}
            return invoke(receiver,forwarded,this_value,false,out);
        },set.function_prototype,set.object_prototype);
    set.function_prototype->properties["call"]=function_call;
    set.function_prototype->properties["apply"]=function_apply;

    auto function=make_native(heap,"Function",1,true,
        [function_prototype=set.function_prototype,object_prototype=set.object_prototype](const std::vector<js_value>&,const js_value&,bool,js_value&out,std::string&,Heap&heap,const NativeInvoke&){
            out=make_native(heap,"anonymous",0,true,[](const std::vector<js_value>&,const js_value&,bool,js_value&result,std::string&,Heap&,const NativeInvoke&){result={};return true;},function_prototype,object_prototype);return true;
        },set.function_prototype,set.function_prototype);

    auto make_error=[&](const std::string&name,const std::shared_ptr<ObjectValue>&prototype){
        return make_native(heap,name,1,true,[name,prototype](const std::vector<js_value>&args,const js_value&receiver,bool construct,js_value&out,std::string&,Heap&heap,const NativeInvoke&){
            if(construct&&receiver.kind==JS_VALUE_OBJECT)out=receiver;else{out.kind=JS_VALUE_OBJECT;out.object=heap.object();out.object->prototype=prototype;}
            const auto message=args.empty()?std::string{}:to_string(args[0]);out.object->properties["name"]=string_value(name);out.object->properties["message"]=string_value(message);out.object->properties["stack"]=string_value(name+(message.empty()?std::string{}:": "+message));return true;
        },set.function_prototype,prototype);
    };
    auto error_prototype=heap.object();error_prototype->prototype=set.object_prototype;error_prototype->properties["name"]=string_value("Error");error_prototype->properties["message"]=string_value("");
    auto type_error_prototype=heap.object();type_error_prototype->prototype=error_prototype;type_error_prototype->properties["name"]=string_value("TypeError");
    auto range_error_prototype=heap.object();range_error_prototype->prototype=error_prototype;range_error_prototype->properties["name"]=string_value("RangeError");
    auto error=make_error("Error",error_prototype),type_error=make_error("TypeError",type_error_prototype),range_error=make_error("RangeError",range_error_prototype);

    for(auto entry:std::vector<std::pair<std::string,js_value>>{{"Object",object},{"Function",function},{"Error",error},{"TypeError",type_error},{"RangeError",range_error}}){set.global->bindings[entry.first]={entry.second,true};set.roots.push_back(entry.second);}
    return set;
}
}
