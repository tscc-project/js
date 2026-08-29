# JS++ implementation roadmap

This roadmap is adaptive. JS0-JS3 are the first concrete implementation path;
later ordering changes when runtime and conformance evidence justifies it.

1. **JS0 - foundation:** C++17 layout, Make build, `include/js.h`, CLI/library
   targets, unit tests, regression runner, handovers, and website.
2. **JS1 - lifecycle:** opaque runtime/value types, creation/destruction,
   evaluation entry point, errors, versioning, allocator/lifetime/thread rules.
3. **JS2 - frontend:** sources, diagnostics, lexer, executable parser, semicolon
   rules, malformed recovery, and independent parse tests.
4. **JS3 - primitive VM:** inspectable bytecode and primitive values, variables,
   arithmetic, comparisons, and completion values.
5. **JS4 - control flow:** blocks, conditions, loops, short-circuiting,
   break/continue, and evaluation-order evidence.
6. **JS5 - functions:** calls, parameters, return, recursion, environments,
   closures, then arrows.
7. **JS6 - objects:** arrays, properties/descriptors, computed access, methods,
   `this`, prototypes, constructors, and `new`.
8. **JS7 - GC:** tracing roots across VM/runtime/native handles, adversarial
   allocation, sanitizers, teardown, and Valgrind.
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
