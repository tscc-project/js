# Minify syntax checkpoint 8: high-value expressions

The lossless syntax view now identifies bounded expression islands for the
ordinary operator, member/call, literal, assignment, conditional, array/object,
function/arrow and `new` token families. An island becomes `Understood` only
when every significant token belongs to the bounded expression vocabulary;
class/module/control keywords and other unsupported syntax leave the candidate
opaque.

The structural delimiter nodes remain opaque barriers. Expression recognition
therefore adds positive ownership information without allowing analysis to
cross an enclosing construct whose semantics have not yet been classified.

The pinned 12-artifact inventory at `privatenumber/minification-benchmarks`
revision `fe89864f` accepts all 27,196,261 bytes and records expression counts
per fixture. The indexed parent assignment keeps the scan linear in retained
tokens/nodes; the complete inventory runs in under three seconds on the
checkpoint host.

Gate: focused positive/negative syntax tests, the full JS++ test wall, the
12-fixture inventory, and an unchanged TSCC tree.

Re-evaluation: statement ownership remains next. It gives these expression
islands exact enclosing control/declaration boundaries before function and
parameter structure is classified.
