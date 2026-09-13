#include <jspp/syntax.h>

#include <cstdlib>
#include <iostream>

namespace {
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
}

int main() {
    jspp::syntax::SyntaxTree tree;
    jspp::syntax::Diagnostic diagnostic;
    check(jspp::syntax::parse_lossless("const answer = 42;", tree, diagnostic) ==
              jspp::syntax::ParseStatus::Success,
          "lossless syntax view rejected source");
    check(tree.source() == "const answer = 42;", "syntax view does not own source");
    check(tree.nodes().size() == 1 && tree.nodes()[0].range.end == tree.source().size(),
          "syntax root does not own complete source");
    std::cout << "JS++ lossless syntax API passed\n";
}
