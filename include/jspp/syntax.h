#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace jspp::syntax {

enum class ParseStatus { Success, SyntaxError };
enum class TokenKind {
    Identifier, Keyword, Number, String, Regex, Template, Punctuator, Comment,
    Hashbang
};
enum class NodeKind {
    Root, Delimited, Expression, Statement, Function, Parameters, Class,
    ImportDeclaration, ExportDeclaration, Opaque
};
enum class SemanticStatus { Opaque, Understood };

struct SourceRange { std::size_t begin = 0, end = 0; };
struct Diagnostic {
    SourceRange range;
    std::size_t line = 1, column = 1;
    std::string message;
};
struct Token {
    TokenKind kind = TokenKind::Punctuator;
    SourceRange range;
};
struct Node {
    NodeKind kind = NodeKind::Opaque;
    SemanticStatus semantics = SemanticStatus::Opaque;
    SourceRange range;
    std::size_t parent = 0;
    std::size_t first_token = 0, last_token = 0;
};
struct Certification {
    std::size_t source_bytes = 0;
    std::size_t token_bytes = 0;
    std::size_t trivia_bytes = 0;
    std::size_t understood_expression_bytes = 0;
    bool exact = false;
    bool deterministic = false;
};

class SyntaxTree {
public:
    std::string_view source() const noexcept { return source_; }
    std::string_view spelling(const Token& token) const noexcept;
    const std::vector<Token>& tokens() const noexcept { return tokens_; }
    const std::vector<Node>& nodes() const noexcept { return nodes_; }

private:
    friend class Scanner;
    friend ParseStatus parse_lossless(std::string, SyntaxTree&, Diagnostic&);
    std::string source_;
    std::vector<Token> tokens_;
    std::vector<Node> nodes_;
};

// Produces a lossless syntax view for analysis consumers. This API is separate
// from JS++'s executable AST: accepting syntax here does not claim that JS++ can
// compile or execute it.
ParseStatus parse_lossless(std::string source, SyntaxTree& output,
                           Diagnostic& diagnostic);

// Certifies token ordering, byte-exact token/trivia reconstruction, node
// ownership and deterministic reparsing. It does not claim executable
// ECMAScript support.
ParseStatus certify_lossless(const SyntaxTree& tree, Certification& result,
                             Diagnostic& diagnostic);

} // namespace jspp::syntax
