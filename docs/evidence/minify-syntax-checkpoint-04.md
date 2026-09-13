# Minify syntax checkpoint 4: lexical goals

The lossless scanner now distinguishes regular-expression literals from division
in expression contexts, recursively tokenizes `${...}` template substitutions,
retains hashbangs, Unicode and escaped identifiers, modern numeric forms, and the
complete current JavaScript punctuator set.

The scanner remains a deliberately bounded minification frontend. Regex lexical
goals use token context; grammar-dependent refinements remain part of the
high-value expression and statement checkpoints rather than expanding JS++ into
a second complete ECMAScript implementation here.

Gate: `make test-syntax` and `make test-unit`.

Re-evaluation: balanced structural regions remain the next dependency. They make
lexical context inspectable and give later opaque barriers exact source bounds.
