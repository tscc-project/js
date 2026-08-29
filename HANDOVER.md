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
