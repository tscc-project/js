# Minify syntax checkpoint 5: balanced structural regions

The syntax view now creates nested, source-ranged nodes for parentheses, brackets,
braces, and `${...}` template substitutions. Each node records its stable parent
and half-open token interval. Mismatched and unclosed delimiters are rejected with
a source diagnostic.

This is structural ownership, not a claim that every brace is semantically
understood. That distinction is made explicit by checkpoint 6's opaque barriers.

Gate: `make test-syntax`, `make test-unit`, and delimiter ownership assertions.

Re-evaluation: explicit opaque regions remain next. Expression grammar must not
consume structure until consumers can distinguish safe transformation islands
from merely balanced source.
