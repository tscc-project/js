# Minify syntax checkpoint 2: separate syntax-view API

Date: 2026-09-13

JS++ now has an experimental C++ lossless-syntax surface distinct from its
executable AST and stable C runtime ABI. `SyntaxTree` owns immutable source bytes
and exposes read-only token/node collections plus bounded spelling access.

The initial implementation creates only a complete-source root. Tokenization and
structural nodes deliberately belong to later checkpoints; accepting source in
this API does not claim that JS++ can compile or execute it.

Re-evaluation: checkpoint 3 remains next because every later structural feature
depends on exact source-owned tokens. Packaging the experimental library is
deferred until its benchmark coverage has been certified.
