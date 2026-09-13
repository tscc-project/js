# Minify syntax checkpoint 6: opaque semantic barriers

Every syntax node now carries an explicit `SemanticStatus`. The root and all
balanced regions produced by the lossless structural scanner are `Opaque` until
a later bounded grammar checkpoint proves that region understood. Consumers can
therefore retain and print exact source without treating delimiter balance as
permission to transform semantics.

The gate reconstructs opaque input byte for byte from token ranges and trivia
gaps. `Understood` is intentionally unused at this checkpoint.

Gate: `make test-syntax`, `make test-unit`, and byte-exact reconstruction.

Re-evaluation: the benchmark inventory remains next. It must measure lexical
acceptance and opaque coverage before expression grammar is prioritized.
