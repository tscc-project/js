# JS++ implementation roadmap

This roadmap is adaptive. JS0-JS3 are the first concrete implementation path;
later ordering changes when runtime and conformance evidence justifies it.

1. **JS0 - foundation — completed 2026-08-30:** C++17 layout, Make build, CLI
   identity, sanitizer target, independent runner, handovers, and website.
2. **JS1 - lifecycle — completed 2026-08-30:** opaque runtime/value types,
   creation/destruction, reserved evaluation entry point, runtime-owned errors,
   versioning and explicit allocator/lifetime/thread rules.
3. **JS2 - frontend — completed 2026-08-30:** attributed sources/diagnostics,
   bounded ECMAScript tokens, executable primitive/declaration syntax, explicit
   semicolon/end rules and independent positive/negative parse tests.
4. **JS3 - primitive VM — completed 2026-08-30:** inspectable stack bytecode,
   primitive values, initialized bindings, arithmetic, equality/order, unary
   operations, completion values, CLI execution and C embedding results.
5. **JS4 - control flow — completed 2026-08-30:** blocks, conditions, classic
   loops, short-circuiting, assignment, conditional expressions,
   break/continue, evaluation-order evidence, const enforcement and a bounded
   instruction budget.
6. **JS5 - functions — completed 2026-08-30:** ordinary/arrow functions, calls,
   parameters, return, recursion, parent-linked block/function environments,
   closures, captured mutation and a bounded call stack.
7. **JS6A - core objects — completed 2026-08-30:** object/array literals,
   named and computed properties, reads/writes, holes/length and aliasing.
8. **JS6B - object model — completed 2026-08-30:** methods, receiver-bound
   calls, `this`, live prototype lookup, constructors and `new`; descriptors
   remain a later object-model refinement.
9. **JS7A - heap foundation — completed 2026-08-30:** engine-owned allocation,
   graph inventory and accounting without collection policy.
10. **JS7B - traced collection — completed 2026-08-30:** runtime-handle roots,
   graph tracing, cycle reclamation, allocation pressure and teardown.
11. **JS7C - execution safe points — completed 2026-08-30:** allocation
   thresholds, active stack/environment roots, compaction and re-entry guards.
12. **JS8A - explicit throw — completed 2026-08-30:** parser, bytecode,
    scope-unwind and call propagation for uncaught primitive throws.
13. **JS8B - handlers:** try/catch/finally, completion interaction, native
   propagation, reporting, and stacks.
10. **JS9 - conversions/built-ins:** centralized coercions and foundational
    Object, Array, String, Number, Boolean, Math, JSON, and Error families.
11. **JS10 - native embedding:** persistent handles, functions, arguments,
    properties, exceptions, user data, finalizers, and safe re-entry.
12. **JS11 - jobs/promises:** host-drained jobs, promises/reactions, then async
    functions; host facilities remain external.
13. **JS12 - modules:** parsing, records, linking, cycles, live bindings, host
    resolver hooks, then dynamic import.
14. **JS13 - broader ECMAScript:** classes, destructuring, spread/rest,
    generators/iterators, symbols, collections, regex, typed arrays, dates, and
    remaining modern families as vertical slices.
15. **JS14 - conformance/hardening:** selected Test262, explicit classifications,
    fuzzing, memory/performance, and supported-platform evidence.
16. **JS15 - packaging/releases:** CLI and libraries, pkg-config/CMake metadata,
    ABI policy, installers, release automation, examples, and truthful docs.

Integration begins after the primitive engine is useful: dual-runtime tscc tests,
independent parsing of tscc output, a shared emitted-JavaScript corpus, then a
syntax-sharing decision gate. Production embedding in tscc remains optional and
requires a concrete bounded feature.

## Preview-qualified next sequence (2026-08-30)

EP0-EP5 has qualified the bounded Linux embedded preview. Next, preserve PC0V as
an external Valgrind task for DeepSeek on Nick's Ubuntu machine; add PC0P
second-platform packaging only in an authorized remote-workflow task; build EP6A
around a pinned selected-Test262 revision; and trial real embedding hosts. The
following feature checkpoint is selected from those results rather than assumed
to be promises or modules in advance.

Compiler coordination status: tscc CP3/TC1 now retains a durable per-file
compilation unit through emission. This changes no JS++ ownership or dependency:
JS++ remains independent, and its first tscc role remains a test runtime after the
primitive engine exists.
CP23/JS8B establishes explicit completion records and preserves thrown values
independently of runtime-error formatting. Handler regions follow in CP25.
CP25/JS8C establishes protected bytecode regions, validated handler targets and
scope/stack unwinding, including cross-frame throws. Surface handlers remain a
post-gate decision.
JS8D implements surface `try`/`catch`, optional lexical catch bindings and
nearest-region dispatch. `finally` remains a separate pending-completion slice.
CP27/JS8E establishes cleanup regions and resumable pending completions before
surface `finally`. Focused VM tests cover retained throws, replacement and
malformed orphan resumes; the public corpus remains 74/74.
CP29/JS8F enables `finally` over normal, return, throw, break and continue paths,
including nested cleanup, catch/finally, cross-frame throws and replacement.
The independent corpus is 86/86.
JS9A centralizes primitive conversion, equality, ordering and property-key
semantics and migrates existing opcodes onto them. The corpus is 98/98;
user-defined object conversion hooks remain coupled to JS10 infrastructure.
JS10A establishes a runtime-owned intrinsic environment, rooted constructor and
prototype identity, and native call/construct callbacks using ordinary VM paths.
The corpus is 102/102; built-in families should now grow only through this layer.
JS10B validates that rule with Object/Function/Error prototypes, call/apply,
error identity/reporting and user conversion hooks. The 114/114 result supports
moving toward Array/String built-ins, but descriptors and complete stack frames
remain prerequisites for broad compatibility claims.
