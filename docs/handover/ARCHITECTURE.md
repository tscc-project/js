# JS++ architecture direction

## Layers

1. Source owns immutable bytes and offset-to-location mapping.
2. Diagnostics own stable attributed syntax/runtime errors.
3. Lexer implements ECMAScript lexical goals deliberately, including the hard
   divisions between regular expressions, division, templates, identifiers,
   escapes, and comments.
4. Parser produces executable syntax with controlled recovery.
5. Bytecode compiler lowers syntax while preserving evaluation order and
   completion semantics.
6. VM executes bytecode against explicit frames, environments, jobs, and roots.
7. Runtime owns values, strings, objects, prototypes, property descriptors,
   functions, exceptions, modules, and built-ins.
8. Collector owns tracing and finalization without exposing internal addresses.
9. Embedding exposes opaque C handles and host callbacks through `js.h`.

## Core invariants

- A parsed construct is not claimed supported until compile, runtime, and error
  behaviour pass where applicable.
- Bytecode is validated before execution and is inspectable in tests.
- Every allocation is either rooted, reachable through a traced owner, or local
  under an explicit no-collection rule.
- Native handles remain valid according to one documented lifetime model.
- Native callbacks cannot unwind C++ exceptions across the C ABI.
- Evaluation order and completion (`normal`, `return`, `throw`, `break`,
  `continue`) are explicit rather than encoded as incidental control flow.
- Hosts own filesystem, network, timers, and module loading policy.
- Optimizations require semantic equivalence tests and measurements.

## Relationship to tscc

The projects share language-domain concepts, not private implementation. tscc
needs lossless TypeScript syntax and compiler recovery; JS++ needs executable
ECMAScript syntax and runtime lowering. Source, token, syntax, or diagnostic reuse
is a later public-API decision gate, not an initial architectural premise.

## JS1 lifecycle contract

`js_runtime` and `js_value` are opaque C handles. Runtime operations are
creator-thread-only; error text belongs to the runtime; values cannot outlive
their runtime and are reclaimed explicitly or at runtime teardown. Null teardown
is harmless. Primitive inspection calls exist before those value kinds become
constructible at JS3. Binary compatibility is not yet promised, but C/C++
implementation details cannot enter `js.h`.

## JS3 execution slice

The frontend lowers to an unoptimized inspectable stack instruction stream.
The VM owns a per-evaluation binding environment and explicit completion stack.
Values are understandable tagged C++ objects; there is no GC because this slice
has no heap object graph. Arithmetic rejects unsupported mixed coercions rather
than silently diverging. `js_eval`, the CLI and focused bytecode tests all consume
the same parser/compiler/VM path.
