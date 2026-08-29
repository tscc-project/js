# JS++ testing and evidence

## Evidence dimensions

Track acceptance/rejection, syntax shape, bytecode shape where contractual,
runtime value, side effects and order, thrown error, embedding ownership,
reference compatibility, memory safety, and timeout/crash status separately.

## Test layers

- Unit tests cover source mapping, tokens, parser nodes, bytecode validation,
  values, objects, environments, roots, and native handles.
- Integration tests compile and execute complete scripts through the public
  engine surface and CLI.
- `js-regression-suite` owns compiler-agnostic black-box cases.
- Node or another named engine is a reference only where ECMAScript compatibility
  is intended; differences require classification.
- Selected Test262 arrives only with a documented harness, exclusions,
  provenance, and pass/fail/unsupported categories.

## High-risk cases

Use getters, counters, coercion hooks, computed properties, nested closures,
prototype mutations, exception/finally paths, recursion, re-entry, forced
collection, allocation failure, malformed delimiters, unusual Unicode, and
timeouts. A top-level arithmetic expression is a smoke test, not engine proof.

## Reproducibility

Record candidate commit, compiler/toolchain, platform, reference-engine version,
configuration, timeouts, and corpus revision. Keep fixtures isolated and output
deterministic. Convert every production defect into a minimized permanent case.

JS2 adds focused AST-shape tests and eight independent `js --check` cases across
precedence, bindings, primitives, comments, malformed delimiters/strings and
unsupported statements. These are parse/error evidence only, not runtime claims.

JS3 expands the independent corpus to 17/17: eight parse/error cases, eight
runtime/error cases and one separately compiled C embedding case. Focused tests
inspect bytecode shape and primitive handle extraction; sanitizer runs pressure
runtime creation, frontend failure/recovery and VM execution.

JS4 expands the independent corpus to 26/26. Nine control-flow cases cover
branching, while/for loops, break/continue, assignment results, const errors,
short-circuit side effects and deterministic termination of an infinite loop.
Focused AST and VM tests additionally inspect jump-bearing bytecode and nested
control-flow completion.

JS5 expands the independent corpus to 36/36. Ten cases cover ordinary calls,
missing arguments, explicit and implicit return, recursion, closures after outer
return, shared captured mutation, expression/block arrows, block shadowing and
non-callable errors. Focused VM tests preserve these alongside prior control-flow
and primitive evidence.

JS6A expands the independent corpus to 46/46. Ten cases cover syntax, object and
array reads/writes, named/computed keys, shorthand, nesting, holes/length,
reference aliasing and invalid primitive bases. Prototype and receiver evidence
must remain in the separate JS6B family.

JS6B expands the independent corpus to 56/56. Ten cases cover method receivers,
computed calls, construction, prototype methods and late mutation, own-property
shadowing, constructor return selection and non-constructor errors.

JS0 establishes `make test` and `make test-sanitize`. The external runner accepts
an explicit candidate and permits an empty corpus only during the scaffold
checkpoint; JS1 removes that exception when lifecycle cases exist.
