# Minify syntax checkpoint 7: benchmark inventory

`make syntax-inventory BENCHMARK_ROOT=/path/to/minification-benchmarks` builds a
small syntax-view client and inventories the 12 benchmark artifacts without
making JS++ depend on that repository. The JSON report records acceptance,
source bytes, tokens, structural nodes, opaque nodes, identifiers, regexes, and
template chunks fixture by fixture.

Acceptance criterion: all 12 installed benchmark artifacts scan successfully.
At this checkpoint all nodes intentionally remain opaque; the report is the
baseline from which checkpoints 8–12 must reduce semantic opacity.

The 2026-09-13 baseline accepts 12/12 artifacts comprising 27,196,261 source
bytes, 3,495,399 tokens, and 429,743 structural nodes. All 429,743 nodes are
opaque by design. The inventory caught and gated a middle-template-chunk nesting
bug before this checkpoint was accepted.

Gate: the 12-fixture inventory, `make test`, and an unchanged TSCC tree.

Re-evaluation: inventory evidence determines the order within the bounded grammar
tranche. Expression coverage remains checkpoint 8 because it unlocks both safe
printing islands and statement parsing; no complete ECMAScript frontend is added.
