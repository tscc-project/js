# Minify syntax checkpoint 9: high-value statements

The syntax view now retains understood statement boundaries inside the root and
brace-owned regions. Semicolon-terminated declarations, expressions and abrupt
statements are partitioned independently of semicolons inside parenthesized
loop headers. Braced blocks receive exact statement ranges while their
enclosing delimiter remains opaque.

Meaningful empty statements are retained explicitly. In particular,
`while (condition);`, `for (...);`, `if (condition);` and `with (value);` do not
disappear into trivia or an adjacent statement.

Class, import, export, case and default starts are not promoted by the generic
semicolon classifier. They remain opaque until their dedicated boundaries are
introduced. Token-to-container and closing-delimiter indices keep ownership
classification linear instead of repeatedly searching the complete node set.

The pinned 12-artifact inventory accepts all fixtures and reports statement
counts per fixture alongside the expression and opaque-node totals.

Gate: focused syntax tests, the full 171-case JS++ wall, the 12-fixture
inventory and an unchanged TSCC tree.

Re-evaluation: function and parameter structure remains next. Statement
ownership now supplies the enclosing bodies needed to distinguish function
headers from ordinary parenthesized expressions.
