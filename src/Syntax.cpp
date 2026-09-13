#include <jspp/syntax.h>

namespace jspp::syntax {

std::string_view SyntaxTree::spelling(const Token& token) const noexcept {
    if (token.range.begin > token.range.end || token.range.end > source_.size())
        return {};
    return std::string_view(source_).substr(token.range.begin,
                                            token.range.end - token.range.begin);
}

ParseStatus parse_lossless(std::string source, SyntaxTree& output,
                           Diagnostic& diagnostic) {
    SyntaxTree candidate;
    candidate.source_ = std::move(source);
    candidate.nodes_.push_back({NodeKind::Root, {0, candidate.source_.size()},
                                0, 0, 0});
    output = std::move(candidate);
    diagnostic = {};
    return ParseStatus::Success;
}

} // namespace jspp::syntax
