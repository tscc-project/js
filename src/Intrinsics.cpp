#include "Intrinsics.h"
#include "Conversion.h"
#include <algorithm>

namespace jspp {
namespace {
js_value string_value(std::string text){js_value value;value.kind=JS_VALUE_STRING;value.string=std::move(text);return value;}
js_value number_value(double number){js_value value;value.kind=JS_VALUE_NUMBER;value.number=number;return value;}
js_value undefined_value(){return {};}
bool object_value(const js_value&value,std::shared_ptr<ObjectValue>&object){if(value.kind==JS_VALUE_OBJECT&&value.object){object=value.object;return true;}return false;}
bool own(const std::shared_ptr<ObjectValue>&object,const std::string&name,js_value&value){auto found=object->properties.find(name);if(found==object->properties.end())return false;value=found->second;return true;}
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

    object.function->properties["create"]=make_native(heap,"create",2,false,
        [](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&error,Heap&heap,const NativeInvoke&){
            if(args.empty()||(args[0].kind!=JS_VALUE_OBJECT&&args[0].kind!=JS_VALUE_NULL)){error="Object.create prototype must be an object or null";return false;}
            out.kind=JS_VALUE_OBJECT;out.object=heap.object();if(args[0].kind==JS_VALUE_OBJECT)out.object->prototype=args[0].object;return true;
        },set.function_prototype,set.object_prototype);
    object.function->properties["getPrototypeOf"]=make_native(heap,"getPrototypeOf",1,false,
        [](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&error,Heap&,const NativeInvoke&){
            std::shared_ptr<ObjectValue>value;if(args.empty()||!object_value(args[0],value)){error="Object.getPrototypeOf requires an object";return false;}
            if(value->prototype){out.kind=JS_VALUE_OBJECT;out.object=value->prototype;}else out.kind=JS_VALUE_NULL;return true;
        },set.function_prototype,set.object_prototype);
    object.function->properties["setPrototypeOf"]=make_native(heap,"setPrototypeOf",2,false,
        [](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&error,Heap&,const NativeInvoke&){
            std::shared_ptr<ObjectValue>value;if(args.size()<2||!object_value(args[0],value)||(args[1].kind!=JS_VALUE_OBJECT&&args[1].kind!=JS_VALUE_NULL)){error="Object.setPrototypeOf requires object and object-or-null prototype";return false;}
            auto prototype=args[1].kind==JS_VALUE_OBJECT?args[1].object:std::shared_ptr<ObjectValue>{};for(auto at=prototype;at;at=at->prototype)if(at==value){error="cyclic prototype value";return false;}value->prototype=prototype;out=args[0];return true;
        },set.function_prototype,set.object_prototype);
    object.function->properties["defineProperty"]=make_native(heap,"defineProperty",3,false,
        [](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&error,Heap&,const NativeInvoke&){
            std::shared_ptr<ObjectValue>value,descriptor;if(args.size()<3||!object_value(args[0],value)||!object_value(args[2],descriptor)){error="Object.defineProperty requires object, key and descriptor";return false;}std::string key;if(!to_property_key(args[1],key)){error="invalid property key";return false;}
            auto existing=value->attributes.find(key);if(existing!=value->attributes.end()&&!existing->second.configurable){error="cannot redefine non-configurable property '"+key+"'";return false;}
            js_value field;PropertyAttributes attributes{false,false,false};if(own(descriptor,"value",field))value->properties[key]=field;else value->properties[key]={};if(own(descriptor,"writable",field))attributes.writable=to_boolean(field);if(own(descriptor,"enumerable",field))attributes.enumerable=to_boolean(field);if(own(descriptor,"configurable",field))attributes.configurable=to_boolean(field);value->attributes[key]=attributes;out=args[0];return true;
        },set.function_prototype,set.object_prototype);
    object.function->properties["keys"]=make_native(heap,"keys",1,false,
        [](const std::vector<js_value>&args,const js_value&,bool,js_value&out,std::string&error,Heap&heap,const NativeInvoke&){
            std::shared_ptr<ObjectValue>value;if(args.empty()||!object_value(args[0],value)){error="Object.keys requires an object";return false;}std::vector<std::string>keys;for(const auto&entry:value->properties){auto attributes=value->attributes.find(entry.first);if(attributes==value->attributes.end()||attributes->second.enumerable)keys.push_back(entry.first);}std::sort(keys.begin(),keys.end());out.kind=JS_VALUE_ARRAY;out.array=heap.array();for(auto&key:keys)out.array->elements.push_back(string_value(std::move(key)));return true;
        },set.function_prototype,set.object_prototype);

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
