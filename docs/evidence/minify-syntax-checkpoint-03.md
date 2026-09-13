# Minify syntax checkpoint 3: lossless lexical tokens

Date: 2026-09-13

The syntax view now owns source-ranged tokens for identifiers, keywords,
numbers, strings, templates, punctuators and comments. Spellings are always
views of the immutable owned source, so decoded executable-AST values cannot
silently replace original syntax. Unterminated strings, templates and block
comments produce attributed syntax diagnostics.

Re-evaluation: lexical-goal handling remains checkpoint 4. Regex-versus-division
and recursive template substitutions must be settled before delimiter structure
can be trusted, so the order of checkpoints 4 and 5 remains unchanged.
