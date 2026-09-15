# JS++ v0.0.1

JS++ v0.0.1 is the first proper release of the independent, embeddable
ECMAScript runtime. It is deliberately an early release: the JavaScript
language implementation is not complete, and v0.0.1 does not claim otherwise.

## What v0.0.1 provides

- Executable ECMAScript parsing, bytecode, VM execution, values, heap and
  garbage collection, exceptions and centralized conversions.
- A frozen embedded-preview surface covering primitives, bindings, structured
  control flow, functions and closures, objects/arrays/prototypes, throw /
  try / catch / finally, conversions, data descriptors/accessors and bounded
  Object, Function, Array, String and Error.
- A versioned C embedding contract through `js.h`: `js_runtime`,
  `js_eval`, `js_value` handles and per-runtime instruction/call-stack limits.
  The API/ABI is version `0.1` (`JS_API_VERSION`, `libjs.so.0`).
- Linux install artifacts: `libjs.a`, `libjs.so.0`, the `js` CLI, `js.h`,
  pkg-config metadata and a CMake imported target.

## Validation for v0.0.1

- Independent black-box regression contract: 171 passed, 0 failed, 0 skipped
  (`js-regression-suite`).
- Unit suites: lifecycle, frontend, VM, heap and standalone-execution lifetime.
- ASan/UBSan clean across lifecycle, frontend, VM and standalone-lifetime.
- External Valgrind confirmation (PC0V): 0 errors, 0 bytes leaked at exit.
- C and C++ package consumers from an isolated install prefix.
- Deterministic malformed-input fuzz (400 mutations) and embedded preview
  host demonstration + soak.

## Explicitly out of scope for v0.0.1

Promises/jobs, ECMAScript modules, Proxy, typed arrays, weak references,
internationalization, browser APIs and Node APIs are excluded from the
embedded-preview contract. Full Test262-style conformance, user-defined
conversion hooks and the broader language/API roadmap remain future work.

Zero known correctness defects within the claimed/certified surface is the
v0.0.1 standard; that is not a claim that the JavaScript implementation is
complete.