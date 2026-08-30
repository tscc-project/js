#include <js.h>

static js_status sum(js_runtime *runtime,const js_value *this_value,
    const js_value *const *arguments,size_t count,void *data,js_value **result) {
    double left=0,right=0;(void)this_value;(void)data;
    if(count!=2||!js_value_get_number(arguments[0],&left)||!js_value_get_number(arguments[1],&right)) {
        js_runtime_set_error(runtime,"sum expects two numbers");
        return JS_STATUS_RUNTIME_ERROR;
    }
    return js_value_new_number(runtime,left+right,result);
}

/* Frozen EP0 exit demonstration. EP3-EP4 replace the remaining marker. */
int main(void) {
    js_runtime *runtime = js_runtime_new();
    js_value *callback = 0, *left = 0, *right = 0, *result = 0;
    const js_value *arguments[2]; double answer = 0;
    if (!runtime) return 1;
    if (js_function_new_native(runtime,"sum",3,sum,0,&callback) ||
        js_value_new_number(runtime,20,&left) || js_value_new_number(runtime,22,&right)) return 2;
    arguments[0]=left;arguments[1]=right;
    if (js_call(runtime,callback,0,arguments,2,&result) ||
        !js_value_get_number(result,&answer) || answer!=42) return 3;
    /* EP3: contain a configured budget failure, then evaluate again. */
    js_value_free(runtime,result);js_value_free(runtime,right);js_value_free(runtime,left);js_value_free(runtime,callback);
    js_runtime_free(runtime);
    return 0;
}
