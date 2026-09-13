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
    check(tree.tokens().size() == 5, "lossless token count");
    check(tree.spelling(tree.tokens()[0]) == "const" &&
          tree.tokens()[0].kind == jspp::syntax::TokenKind::Keyword,
          "keyword spelling not retained");
    check(tree.spelling(tree.tokens()[1]) == "answer" &&
          tree.tokens()[1].kind == jspp::syntax::TokenKind::Identifier,
          "identifier spelling not retained");
    jspp::syntax::SyntaxTree literals;
    check(jspp::syntax::parse_lossless("// c\n'x' /* y */ `z` 1_000n", literals,
                                       diagnostic) == jspp::syntax::ParseStatus::Success,
          "lossless literal tokens rejected");
    check(literals.tokens().size() == 5 &&
          literals.tokens()[0].kind == jspp::syntax::TokenKind::Comment &&
          literals.tokens()[1].kind == jspp::syntax::TokenKind::String &&
          literals.tokens()[2].kind == jspp::syntax::TokenKind::Comment &&
          literals.tokens()[3].kind == jspp::syntax::TokenKind::Template &&
          literals.tokens()[4].kind == jspp::syntax::TokenKind::Number,
          "lossless token families incomplete");
    jspp::syntax::SyntaxTree lexical_goals;
    check(jspp::syntax::parse_lossless(
              "#!/usr/bin/env node\nconst \\u0061=0xCA_FE;let β=.5e+2;"
              "const r=/[/\\\\]/giu;const q=a/b/2;const t=`a${x+`${y}`}z`;a?.b?" "?=1;",
              lexical_goals, diagnostic) == jspp::syntax::ParseStatus::Success,
          "lexical-goal sample rejected");
    std::size_t regexes = 0, templates = 0, hashbangs = 0, slashes = 0;
    for (const auto& token : lexical_goals.tokens()) {
        regexes += token.kind == jspp::syntax::TokenKind::Regex;
        templates += token.kind == jspp::syntax::TokenKind::Template;
        hashbangs += token.kind == jspp::syntax::TokenKind::Hashbang;
        slashes += token.kind == jspp::syntax::TokenKind::Punctuator &&
                   lexical_goals.spelling(token) == "/";
    }
    check(regexes == 1 && templates == 4 && hashbangs == 1 && slashes == 2,
          "regex/division, nested-template, or hashbang lexical goal incorrect");
    jspp::syntax::SyntaxTree invalid;
    check(jspp::syntax::parse_lossless("'unterminated", invalid, diagnostic) ==
              jspp::syntax::ParseStatus::SyntaxError && diagnostic.line == 1,
          "unterminated string accepted");
    std::cout << "JS++ lossless syntax API passed\n";
}
