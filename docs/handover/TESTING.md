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

JS0 establishes `make test` and `make test-sanitize`. The external runner accepts
an explicit candidate and permits an empty corpus only during the scaffold
checkpoint; JS1 removes that exception when lifecycle cases exist.
