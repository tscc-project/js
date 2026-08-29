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

## JS4 control-flow slice

The AST makes blocks, branches, loops and abrupt loop control explicit. The
bytecode compiler resolves jumps and short-circuit/conditional branches while
preserving expression results. Assignment keeps its value on the stack and the
VM distinguishes mutable bindings from constants. Every evaluation has a fixed
one-million-instruction budget so an infinite loop returns a contained runtime
error. Full lexical environment chains and block-local binding scope are JS5
work; JS5 now closes that limitation.

## JS5 functions and environments

Function prototypes own nested bytecode and parameter metadata. Runtime function
values pair a prototype with a retained lexical environment. Each call creates a
child parameter environment, while blocks create child scopes; lookup and
assignment walk parents, so closures survive their defining call and share
captured mutable bindings. Return is explicit bytecode and unwinds active block
scopes. The instruction budget is shared across recursive calls, with a separate
512-frame limit. Shared-pointer ownership is an interim pre-GC model and cycles
remain a JS7 collector concern.

## JS6A core objects and arrays

Object values own string-keyed property maps; arrays own indexed elements plus
named properties. Both are reference values, so assignment preserves aliasing
and strict equality uses identity. Dedicated bytecode constructs literals and
performs reads/writes without repeating base or key evaluation. Missing
properties produce `undefined`; invalid primitive bases are contained runtime
errors. Prototype links exist only as reserved internal representation until
JS6B defines traversal, receiver binding and construction semantics.

## JS6B methods, prototypes and construction

Member-call bytecode retains the evaluated receiver separately from the
resolved function and installs it as the call environment's immutable `this`
binding. Function values own a mutable instance-prototype object. Construct
creates a receiver linked to that object, invokes the function, and selects an
explicit reference return or the receiver. Lookup walks live prototype links;
writes remain own-property writes. This is a deliberately descriptor-free model.
## Central conversions

JS9A centralizes language conversion decisions in `Conversion`. VM opcodes no
longer carry private truthiness, equality, numeric or property-key rules. The
current ToPrimitive object fallback is deliberately bounded until native
prototype methods and user-defined conversion hooks exist.
