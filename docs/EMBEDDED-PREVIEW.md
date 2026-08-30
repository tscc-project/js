# JS++ embedded preview contract plan

The embedded preview is a bounded native product milestone, not a claim of
general ECMAScript compatibility. It is reached when a C or C++ host can create
one runtime, exchange ordinary values, install native functions, execute a
documented language subset under resource limits, inspect exceptions and stacks,
and tear everything down without undefined ownership.

## Contract to freeze

### Lifecycle and ABI

- `js_runtime` and `js_value` remain opaque C handles.
- Runtime operations are confined to the creator thread; values never outlive
  their runtime and every owning handle has an explicit release operation.
- `js_eval` may be called repeatedly on one runtime and reports a typed status
  plus runtime-owned diagnostic text.
- The preview declares ABI/version policy and ships a C header, static and shared
  libraries, and build-system metadata for the supported platforms.

The current API proves runtime creation/destruction, evaluation, value release,
kind inspection, primitive reads, host primitive construction and bounded
object/array construction and own-data access. It is pre-release and may change
while the remaining preview surface is designed.

### Host bridge

EP1 adds deliberate APIs for creating primitive values, constructing objects and
arrays, and reading/writing own data properties/elements. EP2 adds host-to-JS
calls, native callbacks with host data, same-thread re-entry, transferred callback
results, callback errors and retrievable JavaScript exceptions. Callback arguments
are borrowed for the call; a successful callback transfers one owning result
handle to the engine.

### Containment

EP3 exposes per-runtime instruction and call-stack limits plus a cumulative
engine-allocation limit. Instruction/stack exhaustion returns
`JS_STATUS_LIMIT_EXCEEDED`; allocation exhaustion returns
`JS_STATUS_OUT_OF_MEMORY`. Limits can be raised or disabled and the runtime must
evaluate successfully after a contained failure. Evaluation is synchronous, so
there is no asynchronous cancellation API in this preview. JS++ supplies no filesystem, network, timer,
environment or module-loading authority unless the host explicitly provides it.

### Language surface

The preview freezes a versioned matrix rather than saying "JavaScript supported".
Its likely inclusion set is the tested primitives, bindings, control flow,
functions/closures, objects/arrays/prototypes, throw/try/catch/finally,
conversions, descriptors/accessors and bounded Object, Function, Array, String
and Error families. Promises/jobs, ECMAScript modules, proxies, typed arrays,
weak references, internationalization, browser APIs and Node APIs are excluded
unless promoted by their own evidence.

## Executable exit demonstration

A separately compiled C example must create a limited runtime; inject a native
function and structured input; evaluate closures, objects and an array callback;
return a structured result; cross the callback boundary both ways; inspect a
JavaScript Error with deterministic source frames; contain a budget failure and
evaluate successfully afterward; then release everything under ASan and UBSan.

## Definition checkpoints

1. **EP0 - contract inventory:** classify every API, language and packaging item
   as implemented, required, deferred or excluded; freeze the demonstration.
2. **EP1 - host values:** constructors, inspection, object/array properties and
   explicit owning/borrowed-handle rules.
3. **EP2 - calls and callbacks:** JavaScript calls, native registration, host
   data, re-entry, rooting and exception transfer.
4. **EP3 - containment:** public budgets, heap/allocation policy, recovery after
   failure and thread-confinement tests.
5. **EP4 - package and ABI:** shared/static artifacts, symbol visibility,
   versioning, metadata, C/C++ consumers and supported-platform builds.
6. **EP5 - preview candidate:** run the frozen demonstration, regression corpus,
   selected conformance slice, sanitizer/fuzz/soak gates and publish exact limits.

EP0 is frozen. Later checkpoints may split, but may not weaken the exit
demonstration without recording the evidence that forced the change.

## Reassessment after EP0-EP2

Calls/callbacks are no longer the preview blocker. Proceed with EP3 containment
before packaging: the hard-coded instruction budget needs a public per-runtime
contract, heap/allocation limits need deterministic failure, and both must prove
recovery and re-entry safety. EP4 then packages the stabilized API. EP5 runs the
candidate evidence rather than adding another broad language family.
