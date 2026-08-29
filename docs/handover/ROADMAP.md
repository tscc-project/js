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
9. **JS8 - exceptions:** throw/try/catch/finally, completion interaction, native
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

Compiler coordination status: tscc CP3/TC1 now retains a durable per-file
compilation unit through emission. This changes no JS++ ownership or dependency:
JS++ remains independent, and its first tscc role remains a test runtime after the
primitive engine exists.
