#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
${CC:-cc} -std=c11 -I"$root/include" "$root/examples/embedded-preview.c" "$root/build/libjs.a" -lstdc++ -lm -pthread -o "$root/build/embedded-preview"
"$root/build/embedded-preview"
python3 "$root/tools/preview_fuzz.py" "$root/build/js"
echo "JS++ embedded preview demonstration and soak passed"
