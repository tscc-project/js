#include <jspp/syntax.h>

#include <array>
#include <cctype>

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
    const auto fail = [&](std::size_t begin, std::size_t end,
                          std::size_t line, std::size_t column,
                          const char* message) {
        diagnostic = {{begin, end}, line, column, message};
        return ParseStatus::SyntaxError;
    };
    const auto identifier_start = [](unsigned char byte) {
        return std::isalpha(byte) || byte == '_' || byte == '$' || byte >= 0x80;
    };
    const auto identifier_continue = [&](unsigned char byte) {
        return identifier_start(byte) || std::isdigit(byte);
    };
    static const std::array<std::string_view, 43> keywords = {
        "as", "async", "await", "break", "case", "catch", "class", "const",
        "continue", "debugger", "default", "delete", "do", "else", "export",
        "extends", "false", "finally", "for", "from", "function", "get", "if",
        "import", "in", "instanceof", "let", "new", "null", "of", "return",
        "set", "static", "super", "switch", "this", "throw", "true", "try",
        "typeof", "var", "void", "while"
    };
    const auto is_keyword = [&](std::string_view spelling) {
        for (const std::string_view keyword : keywords)
            if (spelling == keyword) return true;
        return spelling == "with" || spelling == "yield";
    };
    const auto add = [&](TokenKind kind, std::size_t begin, std::size_t end) {
        candidate.tokens_.push_back({kind, {begin, end}});
    };
    std::size_t at = 0, line = 1, column = 1;
    const auto advance = [&](char byte) {
        ++at;
        if (byte == '\n') { ++line; column = 1; }
        else ++column;
    };
    while (at < candidate.source_.size()) {
        const unsigned char byte = static_cast<unsigned char>(candidate.source_[at]);
        if (std::isspace(byte)) { advance(candidate.source_[at]); continue; }
        const std::size_t begin = at, begin_line = line, begin_column = column;
        if (identifier_start(byte)) {
            advance(candidate.source_[at]);
            while (at < candidate.source_.size() &&
                   identifier_continue(static_cast<unsigned char>(candidate.source_[at])))
                advance(candidate.source_[at]);
            const std::string_view spelling(candidate.source_.data() + begin, at - begin);
            add(is_keyword(spelling) ? TokenKind::Keyword : TokenKind::Identifier,
                begin, at);
            continue;
        }
        if (std::isdigit(byte) || (byte == '.' && at + 1 < candidate.source_.size() &&
                                  std::isdigit(static_cast<unsigned char>(candidate.source_[at + 1])))) {
            advance(candidate.source_[at]);
            while (at < candidate.source_.size()) {
                const unsigned char next = static_cast<unsigned char>(candidate.source_[at]);
                if (!(std::isalnum(next) || next == '.' || next == '_')) break;
                advance(candidate.source_[at]);
            }
            add(TokenKind::Number, begin, at);
            continue;
        }
        if (byte == '\'' || byte == '"') {
            const char quote = candidate.source_[at];
            advance(candidate.source_[at]);
            bool escaped = false, closed = false;
            while (at < candidate.source_.size()) {
                const char next = candidate.source_[at];
                if (!escaped && next == quote) {
                    advance(next); closed = true; break;
                }
                if (!escaped && (next == '\n' || next == '\r')) break;
                if (!escaped && next == '\\') escaped = true;
                else escaped = false;
                advance(next);
            }
            if (!closed) return fail(begin, at, begin_line, begin_column,
                                     "unterminated string literal");
            add(TokenKind::String, begin, at);
            continue;
        }
        if (byte == '`') {
            advance(candidate.source_[at]);
            bool escaped = false, closed = false;
            while (at < candidate.source_.size()) {
                const char next = candidate.source_[at];
                if (!escaped && next == '`') {
                    advance(next); closed = true; break;
                }
                if (!escaped && next == '\\') escaped = true;
                else escaped = false;
                advance(next);
            }
            if (!closed) return fail(begin, at, begin_line, begin_column,
                                     "unterminated template literal");
            add(TokenKind::Template, begin, at);
            continue;
        }
        if (byte == '/' && at + 1 < candidate.source_.size() &&
            candidate.source_[at + 1] == '/') {
            advance('/'); advance('/');
            while (at < candidate.source_.size() && candidate.source_[at] != '\n')
                advance(candidate.source_[at]);
            add(TokenKind::Comment, begin, at);
            continue;
        }
        if (byte == '/' && at + 1 < candidate.source_.size() &&
            candidate.source_[at + 1] == '*') {
            advance('/'); advance('*');
            bool closed = false;
            while (at < candidate.source_.size()) {
                if (at + 1 < candidate.source_.size() && candidate.source_[at] == '*' &&
                    candidate.source_[at + 1] == '/') {
                    advance('*'); advance('/'); closed = true; break;
                }
                advance(candidate.source_[at]);
            }
            if (!closed) return fail(begin, at, begin_line, begin_column,
                                     "unterminated block comment");
            add(TokenKind::Comment, begin, at);
            continue;
        }
        static const std::array<std::string_view, 29> punctuators = {
            ">>>=", "===", "!==", ">>>", "**=", "&&=", "||=", "?" "?=", "=>",
            "==", "!=", "<=", ">=", "++", "--", "&&", "||", "??", "?.",
            "**", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "...", "&="
        };
        std::size_t length = 0;
        for (const std::string_view punctuator : punctuators)
            if (punctuator.size() > length &&
                candidate.source_.compare(at, punctuator.size(), punctuator) == 0)
                length = punctuator.size();
        if (!length) length = 1;
        for (std::size_t count = 0; count < length; ++count)
            advance(candidate.source_[at]);
        add(TokenKind::Punctuator, begin, at);
    }
    candidate.nodes_.push_back({NodeKind::Root, {0, candidate.source_.size()},
                                0, 0, candidate.tokens_.size()});
    output = std::move(candidate);
    diagnostic = {};
    return ParseStatus::Success;
}

} // namespace jspp::syntax
