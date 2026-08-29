# JS++ development handover

This is the entry point for work on JS++, the independent embeddable JavaScript
engine in this repository. The repository is intentionally at pre-implementation
checkpoint JS0. Do not infer working engine behaviour from planned API examples.

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

This is direction, not yet a frozen ABI. Before implementing it, define error
ownership, value lifetime, runtime affinity, allocator behaviour, thread safety,
native re-entry, and versioning. Never document planned calls as released.

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

The first integration is test-only: the tscc regression harness will eventually
run eligible emitted JavaScript in both Node and JS++. Node remains the initial
compatibility oracle. JS++ gaps must not be mislabeled as tscc failures.

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
- `docs/handover/ROADMAP.md`: adaptive JS0-JS15 implementation plan.
- `docs/handover/TESTING.md`: external evidence and conformance methodology.

These are living infrastructure. Consolidate durable decisions rather than
appending an unstructured diary.
