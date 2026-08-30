#include <js.h>
#include <stdio.h>

static js_status sum(js_runtime *runtime,const js_value *this_value,
    const js_value *const *arguments,size_t count,void *data,js_value **result) {
    double left=0,right=0;(void)this_value;(void)data;
    if(count!=2||!js_value_get_number(arguments[0],&left)||!js_value_get_number(arguments[1],&right)) {
        js_runtime_set_error(runtime,"sum expects two numbers");
        return JS_STATUS_RUNTIME_ERROR;
    }
    return js_value_new_number(runtime,left+right,result);
}

/* Frozen embedded preview exit demonstration. */
int main(void) {
    js_runtime *runtime = js_runtime_new();
    js_value *callback = 0, *left = 0, *right = 0, *input = 0, *values = 0,
        *exception = 0, *result = 0;
    const js_value *arguments[2]; double answer = 0;
    if (!runtime) return 1;
    if (js_function_new_native(runtime,"sum",3,sum,0,&callback) ||
        js_value_new_number(runtime,20,&left) || js_value_new_number(runtime,22,&right)) return 2;
    arguments[0]=left;arguments[1]=right;
    if (js_call(runtime,callback,0,arguments,2,&result) ||
        !js_value_get_number(result,&answer) || answer!=42) return 3;
    js_value_free(runtime,result);result=0;
    if (js_object_new(runtime,&input) || js_array_new(runtime,&values) ||
        js_array_set(runtime,values,0,left) || js_array_set(runtime,values,1,right) ||
        js_object_set(runtime,input,"values",6,values) ||
        js_runtime_set_global(runtime,"input",5,input) ||
        js_runtime_set_global(runtime,"sum",3,callback)) return 4;
    if (js_eval(runtime,"function make(offset){return function(reading){const mapped=reading.values.map(function(value){return value+offset;});return sum(mapped[0],mapped[1]);};} make(0)(input);",&result) != JS_STATUS_OK) {
        fprintf(stderr,"embedded preview evaluation failed: %s\n",js_runtime_last_error(runtime));return 4;
    }
    if (!js_value_get_number(result,&answer) || answer!=42) return 4;
    js_value_free(runtime,result);result=0;
    if (js_eval(runtime,"throw new Error('preview');",&result)!=JS_STATUS_RUNTIME_ERROR ||
        js_runtime_get_exception(runtime,&exception)!=JS_STATUS_OK ||
        js_value_get_kind(exception)!=JS_VALUE_OBJECT) return 5;
    js_value_free(runtime,exception);exception=0;
    if (js_runtime_set_instruction_limit(runtime,32) ||
        js_eval(runtime,"while(true){}",&result)!=JS_STATUS_LIMIT_EXCEEDED ||
        js_runtime_set_instruction_limit(runtime,1000) ||
        js_eval(runtime,"6*7",&result)!=JS_STATUS_OK) return 6;
    js_value_free(runtime,result);result=0;
    for (int i=0;i<250;++i) {
        if (js_eval(runtime,"make(0)(input);",&result)!=JS_STATUS_OK ||
            !js_value_get_number(result,&answer) || answer!=42) return 7;
        js_value_free(runtime,result);result=0;
    }
    js_value_free(runtime,values);js_value_free(runtime,input);js_value_free(runtime,right);js_value_free(runtime,left);js_value_free(runtime,callback);
    js_runtime_free(runtime);
    return 0;
}
