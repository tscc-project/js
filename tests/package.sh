#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
make -C "$root" install DESTDIR="$stage" PREFIX=/usr
cat > "$stage/consumer.c" <<'EOF'
#include <js.h>
int main(void) {
    js_runtime *runtime = js_runtime_new();
    if (!runtime || js_api_version() != JS_API_VERSION) return 1;
    js_runtime_free(runtime);
    return 0;
}
EOF
cat > "$stage/consumer.cpp" <<'EOF'
#include <js.h>
int main() {
    js_runtime *runtime = js_runtime_new();
    js_value *value = nullptr;
    const auto status = js_eval(runtime, "40 + 2", &value);
    double result = 0;
    const bool ok = status == JS_STATUS_OK && js_value_get_number(value, &result) && result == 42;
    js_value_free(runtime, value);
    js_runtime_free(runtime);
    return ok ? 0 : 1;
}
EOF
${CC:-cc} -std=c11 -I"$stage/usr/include" "$stage/consumer.c" "$stage/usr/lib/libjs.a" -lstdc++ -lm -pthread -o "$stage/static-consumer"
${CXX:-c++} -std=c++17 -I"$stage/usr/include" "$stage/consumer.cpp" -L"$stage/usr/lib" -Wl,-rpath,"$stage/usr/lib" -ljs -pthread -o "$stage/shared-consumer"
"$stage/static-consumer"
"$stage/shared-consumer"
test "$(nm -D --defined-only "$stage/usr/lib/libjs.so.0" | awk '{print $3}' | grep -Ev '^(js_.*@@JSPP_0\.1|JSPP_0\.1)$' | wc -l)" -eq 0
test -f "$stage/usr/lib/pkgconfig/jspp.pc"
test -f "$stage/usr/lib/cmake/JSpp/JSppConfig.cmake"
echo "JS++ installed package passed C and C++ consumers"
