# JS++ and tscc product boundary

**Status:** settled at CP1 on 2026-08-30.

JS++ is an independently useful embeddable ECMAScript engine. It owns executable
JavaScript parsing, bytecode, runtime values and objects, environments, closures,
exceptions, garbage collection, jobs, modules, built-ins, and its public C API.

tscc owns TypeScript compiler syntax, symbols, binding, types, diagnostics,
projects, module resolution, lowering, and JavaScript emission. CommonJS lowering
stays in tscc; ECMAScript module execution belongs here.

Normal tscc compilation does not require or execute through JS++. The first
integration is an explicit test adapter that runs eligible emitted JavaScript in
Node and JS++. Private implementation headers never cross repositories. A shared
syntax facility or production embedding dependency requires a later evidence-backed
checkpoint and a versioned public surface.
