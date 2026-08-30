#include <js.h>

/* Frozen EP0 exit demonstration. EP1-EP4 replace the markers with public API. */
int main(void) {
    js_runtime *runtime = js_runtime_new();
    js_value *result = 0;
    if (!runtime) return 1;
    /* EP1: create structured input and inspect a structured result. */
    /* EP2: install/re-enter a native callback and inspect an exception. */
    /* EP3: contain a configured budget failure, then evaluate again. */
    if (js_eval(runtime, "[20,21].map(x=>x+1).join(',')", &result) != JS_STATUS_OK) return 2;
    js_value_free(runtime, result);
    js_runtime_free(runtime);
    return 0;
}
