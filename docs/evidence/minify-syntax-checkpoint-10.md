# Minify syntax checkpoint 10: functions and parameters

The lossless syntax view now retains bounded ordinary-function and arrow nodes,
with separate understood parameter regions. Named and anonymous functions,
generator markers, parenthesized parameters, single-identifier arrow
parameters, defaults, rest and destructuring are retained without requiring the
executable JS++ parser or VM to support additional syntax.

Braced bodies use their validated delimiter range. Expression-bodied arrows
stop at the comma or semicolon in their owning delimiter container. Function
bodies and their structural delimiters remain opaque barriers; recognizing the
function boundary is not permission to optimize through unsupported contents.

The pinned 12-fixture inventory finds exactly 43,480 function nodes and 43,480
parameter regions across 27,196,261 bytes. Single-identifier arrow parameters
do not require synthetic opaque delimiters, so the original 429,743 opaque-node
baseline is unchanged. Indexed open/close lookups avoid repeated delimiter
searches.

Gate: focused function/parameter tests, the full 171-case JS++ wall, ASan/UBSan,
the 12-fixture inventory and an unchanged TSCC tree.

Re-evaluation: class and module boundaries remain next. They are the last major
top-level ownership families needed before lossless frontend certification.
