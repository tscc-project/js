# Minify syntax checkpoint 11: class and module boundaries

The syntax view now retains understood boundary nodes for classes and static
import/export declarations. Class ranges terminate at their validated body
delimiter while the class body remains opaque. Static imports are distinguished
from dynamic `import(...)` and `import.meta`; property access such as
`object.class` is not mistaken for a class boundary.

Semicolon-terminated module declarations, exported functions/classes and final
semicolonless declarations receive bounded ranges. This is source ownership
only: JS++ does not gain class or module execution, linking or host resolution.

The checkpoint inventory initially recorded 16 class, 6 import and 8 export
candidates. Checkpoint 12's ownership certification exposed those import/export
candidates as property spellings such as `_a.export`; the classifier now
requires static module declarations at the source root. The certified corpus
therefore contains 15 class boundaries and no static imports/exports, which is
consistent with these bundled artifacts. The 429,743 opaque delimiter/root
barriers are unchanged.

Gate: focused class/static-module/dynamic-import tests, the full 171-case JS++
wall, the 12-fixture inventory and an unchanged TSCC tree.

Re-evaluation: the syntax families required by Batch B now have explicit
boundaries. Checkpoint 12 should certify coverage, overlap/gap invariants,
determinism and byte-exact reconstruction rather than add another grammar
family.
