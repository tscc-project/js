# Minify syntax checkpoint 12: lossless frontend certification

`certify_lossless` now provides an explicit frontend gate. It rejects invalid or
overlapping token ranges, reconstruction differences, invalid node/token spans,
self-parenting, child ranges that escape their parent and nondeterministic
reparsing. Certification reports source, token, trivia and conservatively
understood expression bytes without claiming that structurally bounded
statements/functions/classes are semantically executable.

The first corpus run failed usefully. Parenthesized arrow bodies could absorb a
closing delimiter owned by their caller, and property spellings such as
`_a.export` were classified as module declarations. Arrow ranges are now capped
at their enclosing delimiter and static import/export declarations must occur at
the source root. Dynamic import and import.meta remain expression syntax.

At pinned `privatenumber/minification-benchmarks` revision `fe89864f`, all 12
fixtures certify:

- source bytes: 27,196,261;
- ordered token bytes: 21,357,308;
- exact trivia-gap bytes: 5,838,953;
- tokens: 3,495,399;
- opaque root/delimiter barriers: 429,743;
- understood expression nodes: 441,996 covering 13,853,768 source bytes;
- understood statement boundaries: 310,838;
- functions and parameter regions: 43,480 each;
- class boundaries: 15;
- static imports/exports: 0/0 in the bundled fixtures;
- certified fixtures: 12/12.

Token bytes plus trivia bytes equal every fixture's source size. Reparsing every
fixture produces the same token and node kinds, spans, parents and semantic
statuses. The full inventory remains under three seconds on the checkpoint host.

Gate: focused certification tests, 12/12 pinned fixture certification, the full
171-case JS++ regression wall, ASan/UBSan, clean rebuild and an unchanged TSCC
tree.

Re-evaluation: Batch B is complete. The next coherent batch is checkpoints
13-17 in Minify++: the syntax-view adapter, expression IR, statement IR, pattern
and declaration IR, then scope/reference integration. Opaque barriers remain
fail-closed throughout that adapter work.
