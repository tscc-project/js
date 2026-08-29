# JS++ development handover

This is the entry point for work on JS++, the independent embeddable JavaScript
engine in this repository. JS0 is complete: the C++17 build, CLI identity,
sanitizer target and independent empty-corpus runner are executable. Evaluation
and the public ABI remain JS1 work; do not infer engine behaviour from scaffolding.

Coordinated status (2026-08-30): tscc CP3/TC1 now retains a durable per-file
compilation unit through emission. This is compiler-internal ownership work; it
moves nothing into JS++, creates no runtime dependency, and leaves test-only
integration as the first planned relationship after a primitive engine exists.

## Identity and artifacts

- Product/repository identity: JS++.
- Implementation language: C++17 initially.
- Public C header: `js.h`.
- Static library: `libjs.a`.
- Shared library: `libjs.so` on Linux, with platform-appropriate equivalents.
- CLI/shell/script runner: `js`.
- License: MIT.

The distinctive project name and deliberately plain embedding artifacts are an
intentional separation. Revisit install-time collisions during packaging; do not
rename the public surface casually.

## Product boundary

JS++ owns executable ECMAScript parsing, bytecode, runtime semantics, values,
objects, prototypes, lexical environments, closures, exceptions, garbage
collection, jobs, modules, built-ins, and native embedding.

JS++ does not own TypeScript binding, types, diagnostics, project compilation, or
TypeScript-to-JavaScript lowering. Those belong to tscc. JS++ must remain useful
without tscc, and tscc ordinary compilation must not require executing input in
JS++.

## Planned architecture

```text
source
→ source locations and diagnostics
→ ECMAScript lexer
→ executable syntax tree
→ bytecode compiler
→ virtual machine
→ values / objects / environments / built-ins
→ host embedding boundary
```

Begin with clear tagged values and inspectable bytecode. Do not select NaN boxing,
JIT compilation, generational GC, or other optimizations before profiles and
correctness evidence justify them.

## Public API rule

`include/js.h` is a stable C-compatible boundary with opaque types. C++ internals
must not leak into it. The first lifecycle slice is expected to resemble:

```c
js_runtime *js_runtime_new(void);
void js_runtime_free(js_runtime *runtime);
int js_eval(js_runtime *runtime, const char *source, js_value *result);
```

JS1 freezes this initial source-level API, while binary compatibility remains
pre-release. Runtime error strings are runtime-owned. Values are opaque,
runtime-affine handles explicitly freed with `js_value_free` or reclaimed at
runtime teardown. A runtime is confined to its creator thread. Allocation uses
the process C++ allocator for now; failures and C++ exceptions are contained.
Native callbacks and re-entry do not exist yet. `js_eval` reports unsupported
until JS3.

JS2 adds attributed lexer diagnostics and an executable AST for primitive
literals, identifiers, initialized declarations, expression statements,
parentheses, unary operations, arithmetic, equality and ordering. `js --check`
is the external parse contract. JS3 lowers exactly this grammar to inspectable
stack bytecode and executes number, string, boolean, null and undefined values,
bindings, numeric arithmetic, same-type string concatenation, equality, ordering
and unary operations. JS4 adds blocks, if/else, while and classic for loops,
break/continue, assignment, short-circuit logical operators, conditional
expressions and a one-million-instruction evaluation budget. Const assignment is
rejected. Lexical block environments begin with JS5; calls, objects, coercive
mixed arithmetic and templates remain explicitly unsupported.

JS5 adds ordinary and arrow functions, calls, parameters, explicit/implicit
return, recursion, parent-linked function/block environments and closures that
retain and mutate captured bindings after their defining call returns. Calls
share the evaluation instruction budget and have a 512-frame safety limit.
Function values are identifiable through `js_value_kind`, but native calls and
public invocation handles remain JS10 work. Shared ownership is provisional
until JS7 replaces it with traced engine-managed lifetime.

JS6A adds core object and array values, literals, named/computed property reads
and writes, shorthand properties, array holes/length and reference aliasing.
Property bytecode evaluates the base and key once and assignments leave the
assigned value as their completion. Descriptors, methods, `this`, prototype
traversal and `new` are deliberately reserved for JS6B.

JS6B completes that bounded object-model slice with object-literal methods,
receiver-preserving member calls, `this`, function prototype objects, live
prototype lookup, constructors and JavaScript constructor-return selection.
Descriptors/accessors, classes, built-in prototype families, `instanceof` and
general member-expression constructor targets remain future work.

JS7A establishes one engine-owned allocation registry for environments,
functions, objects and arrays. Every VM allocation now passes through `Heap`,
which owns allocation accounting and weak graph inventory while preserving the
existing 56/56 behavior. Collection remains deliberately inactive until JS7B.

JS7B activates root tracing from runtime handles and graph traversal across VM
values, properties, prototypes, functions, closures and environments. Sweep
breaks unreachable shared cycles before releasing them. Focused tests retain
live handles across collections and reclaim closure/environment and self-object
cycles under repeated allocation pressure.

JS7C adds allocation-threshold safe points during top-level execution, roots the
active operand stack and lexical environment, compacts expired heap inventory,
and guards collector re-entry. Recursive calls collect only after control
returns to a top-level safe point so caller-frame temporaries cannot be missed.

Coordination update: tscc CP4/TC2 now uses one production `ProgramGraph`. It does
not link or invoke JS++, and the test-only-first boundary is unchanged.

## Development standard

```text
define one ECMAScript/runtime slice
→ add independent observable cases
→ lex and parse it completely
→ compile inspectable bytecode
→ execute and test errors/evaluation order
→ exercise embedding ownership when applicable
→ sanitizers and allocation pressure
→ reconcile feature matrix, handovers, and website
```

Parser acceptance is not language support. Support requires every applicable
stage through runtime and errors. Side-effect count/order, closures, exceptions,
coercions, prototypes, roots, finalization, and malformed input are high-risk
families.

## Regression suite

The sibling `js-regression-suite` owns the independent black-box contract. Keep
engine unit tests close to implementation and externally observable compatibility
cases outside it. Later selected Test262 use must retain explicit classifications
and licensing/provenance.

## tscc integration

The first integration is test-only and is now implemented at CP9/INT0: the tscc
regression harness runs six eligible emitted programs independently in Node and
JS++. Explicit expected completion values prevent equal-but-wrong agreement.
Node remains the initial compatibility oracle, and JS++ gaps must not be
mislabeled as normal tscc failures.

No existing tscc implementation moves here initially. A public syntax facility is
considered only after both parsers exist and comparative evidence proves a stable
common contract. See `docs/handover/ROADMAP.md` and tscc's
`docs/handover/COMPILER-JS-ROADMAP.md`.

## Checkpoints and public actions

A checkpoint is an evidence-backed local baseline, not a release or compatibility
claim. Report exact source/bytecode/runtime/API coverage, tests, native-safety
evidence, unsupported behaviour, and repository residue. Do not commit, push,
tag, release, deploy, or publish without explicit authorization.

## Deeper handovers

- `docs/handover/ARCHITECTURE.md`: component ownership and invariants.
- `docs/handover/PRODUCT-BOUNDARY.md`: settled CP1 relationship with tscc.
- `docs/handover/ROADMAP.md`: adaptive JS0-JS15 implementation plan.
- `docs/handover/TESTING.md`: external evidence and conformance methodology.
- `docs/feature-matrix.json`: executable per-stage language/runtime status.

Run `python3 tools/check_feature_matrix.py` whenever checkpoint status changes.

These are living infrastructure. Consolidate durable decisions rather than
appending an unstructured diary.
## JS8A / CP21 explicit throw (2026-08-30)

Explicit `throw` now has parser and bytecode identity, unwinds lexical scopes,
propagates through calls and surfaces uncaught primitive values as deterministic
runtime errors. `try`/`catch`/`finally`, exception objects and native exception
APIs remain future work. The independent corpus is 62/62.
## JS8B / CP23 completion records (2026-08-30)

The VM now carries explicit Normal, Return, Throw, Break and Continue completion
kinds. Throws retain their `js_value` across function frames and only the public
execution boundary formats an uncaught runtime error. Break/continue remain
compiled jumps until handler/finally evidence requires interception.
## JS8C / CP25 handler regions (2026-08-30)

Bytecode can now describe protected instruction ranges with handler targets,
scope depth and operand-stack depth. The VM selects the nearest region, unwinds
lexical environments and stack state, roots the pending thrown value by moving
it onto the handler stack, and accepts throws propagated from calls, methods and
constructors. Focused tests cover local/cross-frame dispatch and malformed
metadata. No `try` surface syntax is accepted yet.
## JS8D try/catch (2026-08-30)

`try`/`catch` is now source-facing. Catch bindings are optional and lexical;
the compiler emits protected regions and handler prologues, while the VM routes
local and cross-frame thrown values to the smallest containing region. Nested
catches, rethrows, normal fallthrough, object throws and returns from catch are
covered. `finally` remains deliberately rejected until pending completion of
return/break/continue can be intercepted and resumed correctly.

## CP27 / JS8E pending completions (2026-08-30)

Bytecode now owns cleanup regions and `completion.resume`. The VM suspends
throws and returns across cleanup code, reconstructs lexical/operand state,
resumes exactly once, and permits a cleanup return/throw to replace the pending
completion. Surface `finally` remains disabled; the corpus stays 74/74.

## CP29 / JS8F finally (2026-08-30)

`try/finally` and `try/catch/finally` now run through cleanup regions for normal,
return, throw, break and continue completions. Cleanup executes once, nested and
cross-frame paths are retained, and a return/throw from `finally` replaces the
pending completion. The independent corpus is 86/86. Exception objects, stack
traces and native exception APIs remain later work.

## JS9A centralized conversions (2026-08-30)

`Conversion` now owns ToBoolean, bounded ToPrimitive, ToNumber, ToString,
ToPropertyKey, array-index recognition, strict/abstract equality and relational
comparison. Unary/arithmetic/addition/comparison/property opcodes use that shared
layer. The corpus is 98/98. User-defined conversion hooks await JS10.

## JS10A intrinsic/native framework (2026-08-30)

Each runtime now owns one persistent intrinsic environment rooted through GC.
Native functions share normal call/construct bytecode paths, carry `name`,
`length`, constructibility and prototype links, and may re-enter the common
invoker. `Object` is the first constructor proving call and construction paths.
The corpus is 102/102. Broader Object/Function/Error behavior follows in JS10B.

## JS10B Object, Function and Error foundations (2026-08-30)

`Object.prototype` supplies native `valueOf`/`toString`; ordinary object literals
inherit it. `Function.prototype.call` and bounded array-only `apply` re-enter the
common invoker. `Error`, `TypeError` and `RangeError` create prototype-backed
objects with name, message and deterministic initial stack text; catch/rethrow
preserves identity and uncaught formatting recognizes errors. VM conversions now
dispatch user `valueOf`/`toString` for numeric/string/property-key operations.
The corpus is 114/114. Full descriptors, `instanceof`, dynamic Function source,
complete stacks and exception-producing conversion hooks remain future work.

## JS10C descriptors and prototype reflection (2026-08-29)

Ordinary objects now retain data-property writable/enumerable/configurable flags.
`Object.create`, `getPrototypeOf`, `setPrototypeOf`, `defineProperty` and `keys`
provide a bounded reflection surface, and `instanceof` walks object, array and
function prototype chains through its own opcode. Cyclic prototype mutation and
invalid/non-callable operands fail deterministically. The corpus is 124/124.
Accessors, primitive boxing and complete descriptor invariants remain deferred.

## JS10D Array and String foundations (2026-08-29)

Array literals and constructed arrays share `Array.prototype`; the constructor,
`isArray`, push/pop/join/slice/indexOf/map all use the native invoker and heap.
Primitive strings expose length, indexed characters and String prototype
slice/includes/indexOf, while `String(value)` uses centralized conversion. The
corpus is 138/138. Sparse-array semantics, iterators, callbacks beyond map,
Unicode indexing and boxed String objects remain deferred.

## JS10E shared descriptors and accessors (2026-08-29)

Objects, arrays and functions now retain the same descriptor metadata shape and
GC traces accessor functions. `defineProperty` accepts data or accessor
descriptors across those values, `getOwnPropertyDescriptor` reflects them, and
ordinary object getter/setter invocation uses the common VM invoker. Read-only
flags are enforced on named array/function properties. The corpus is 148/148.
Indexed array descriptors and fully unified exotic-object operations remain
bounded future work.

## JS10F abrupt conversion and native errors (2026-08-29)

Thrown completions from valueOf/toString, property-key conversion, accessors and
native callback re-entry now dispatch through catch/finally instead of becoming
undefined, NaN or generic host failures. Native intrinsic validation failures
become catchable TypeError objects; invalid Array lengths become RangeError,
with deterministic initial stack text and identity preserved across rethrow.
The corpus is 158/158. Source-position stack frames and a general VM-wide
throwing error factory remain future work.
