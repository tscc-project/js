#include <jspp/syntax.h>

#include <array>
#include <cctype>
#include <set>
#include <utility>

namespace jspp::syntax {

std::string_view SyntaxTree::spelling(const Token& token) const noexcept {
    if (token.range.begin > token.range.end || token.range.end > source_.size())
        return {};
    return std::string_view(source_).substr(token.range.begin,
                                            token.range.end - token.range.begin);
}

class Scanner {
public:
    Scanner(SyntaxTree& tree, Diagnostic& diagnostic)
        : tree_(tree), diagnostic_(diagnostic) {}

    ParseStatus scan() {
        if (starts_with("#!")) scan_line_comment(TokenKind::Hashbang);
        return scan_code(false) ? ParseStatus::Success : ParseStatus::SyntaxError;
    }

private:
    bool scan_code(bool template_expression) {
        std::size_t braces = 0;
        while (at_ < source().size()) {
            const unsigned char byte = peek();
            if (std::isspace(byte)) { advance(); continue; }
            if (template_expression && byte == '}' && braces == 0) return true;

            const std::size_t begin = at_;
            const std::size_t begin_line = line_;
            const std::size_t begin_column = column_;
            if (identifier_start()) {
                if (!scan_identifier())
                    return fail(begin, at_, begin_line, begin_column,
                                "invalid Unicode escape in identifier");
                const std::string_view word = slice(begin, at_);
                add(is_keyword(word) ? TokenKind::Keyword : TokenKind::Identifier,
                    begin, at_);
                continue;
            }
            if (std::isdigit(byte) ||
                (byte == '.' && std::isdigit(peek(1)))) {
                scan_number();
                add(TokenKind::Number, begin, at_);
                continue;
            }
            if (byte == '\'' || byte == '"') {
                if (!scan_string(static_cast<char>(byte)))
                    return fail(begin, at_, begin_line, begin_column,
                                "unterminated string literal");
                add(TokenKind::String, begin, at_);
                continue;
            }
            if (byte == '`') {
                if (!scan_template())
                    return fail(begin, at_, begin_line, begin_column,
                                "unterminated template literal");
                continue;
            }
            if (starts_with("//")) { scan_line_comment(TokenKind::Comment); continue; }
            if (starts_with("/*")) {
                if (!scan_block_comment())
                    return fail(begin, at_, begin_line, begin_column,
                                "unterminated block comment");
                continue;
            }
            if (byte == '/' && regex_allowed()) {
                if (!scan_regex())
                    return fail(begin, at_, begin_line, begin_column,
                                "unterminated regular expression literal");
                add(TokenKind::Regex, begin, at_);
                continue;
            }
            const std::size_t length = punctuator_length();
            for (std::size_t i = 0; i < length; ++i) advance();
            add(TokenKind::Punctuator, begin, at_);
            if (template_expression) {
                if (slice(begin, at_) == "{") ++braces;
                else if (slice(begin, at_) == "}" && braces) --braces;
            }
        }
        return !template_expression;
    }

    bool scan_identifier() {
        do {
            if (peek() == '\\') {
                if (!scan_unicode_escape()) return false;
            } else {
                advance();
            }
        } while (at_ < source().size() && identifier_part());
        return true;
    }

    bool scan_unicode_escape() {
        advance();
        if (peek() != 'u') return false;
        advance();
        if (peek() == '{') {
            advance();
            std::size_t digits = 0;
            while (std::isxdigit(peek())) { advance(); ++digits; }
            if (!digits || peek() != '}') return false;
            advance();
            return true;
        }
        for (int i = 0; i < 4; ++i) {
            if (!std::isxdigit(peek())) return false;
            advance();
        }
        return true;
    }

    void scan_number() {
        if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X' ||
                             peek(1) == 'b' || peek(1) == 'B' ||
                             peek(1) == 'o' || peek(1) == 'O')) {
            advance(); advance();
            while (std::isalnum(peek()) || peek() == '_') advance();
            return;
        }
        if (peek() == '.') advance();
        while (std::isdigit(peek()) || peek() == '_') advance();
        if (peek() == '.') {
            advance();
            while (std::isdigit(peek()) || peek() == '_') advance();
        }
        if (peek() == 'e' || peek() == 'E') {
            advance();
            if (peek() == '+' || peek() == '-') advance();
            while (std::isdigit(peek()) || peek() == '_') advance();
        }
        if (peek() == 'n') advance();
    }

    bool scan_string(char quote) {
        advance();
        bool escaped = false;
        while (at_ < source().size()) {
            const char byte = static_cast<char>(peek());
            if (!escaped && byte == quote) { advance(); return true; }
            if (!escaped && (byte == '\n' || byte == '\r')) return false;
            if (!escaped && byte == '\\') escaped = true;
            else escaped = false;
            advance();
        }
        return false;
    }

    bool scan_template() {
        std::size_t chunk = at_;
        advance();
        bool escaped = false;
        while (at_ < source().size()) {
            if (!escaped && peek() == '`') {
                advance(); add(TokenKind::Template, chunk, at_); return true;
            }
            if (!escaped && peek() == '$' && peek(1) == '{') {
                advance(); advance();
                add(TokenKind::Template, chunk, at_);
                if (!scan_code(true) || peek() != '}') return false;
                chunk = at_;
                advance();
                escaped = false;
                continue;
            }
            if (!escaped && peek() == '\\') escaped = true;
            else escaped = false;
            advance();
        }
        return false;
    }

    bool scan_regex() {
        advance();
        bool escaped = false, character_class = false;
        while (at_ < source().size()) {
            const char byte = static_cast<char>(peek());
            if (!escaped && (byte == '\n' || byte == '\r')) return false;
            if (!escaped && byte == '[') character_class = true;
            else if (!escaped && byte == ']') character_class = false;
            else if (!escaped && byte == '/' && !character_class) {
                advance();
                while (identifier_part()) advance();
                return true;
            }
            if (!escaped && byte == '\\') escaped = true;
            else escaped = false;
            advance();
        }
        return false;
    }

    void scan_line_comment(TokenKind kind) {
        const std::size_t begin = at_;
        advance(); advance();
        while (at_ < source().size() && peek() != '\n' && peek() != '\r') advance();
        add(kind, begin, at_);
    }

    bool scan_block_comment() {
        const std::size_t begin = at_;
        advance(); advance();
        while (at_ < source().size()) {
            if (starts_with("*/")) {
                advance(); advance(); add(TokenKind::Comment, begin, at_); return true;
            }
            advance();
        }
        return false;
    }

    bool regex_allowed() const {
        for (auto it = tree_.tokens_.rbegin(); it != tree_.tokens_.rend(); ++it) {
            if (it->kind == TokenKind::Comment || it->kind == TokenKind::Hashbang) continue;
            const std::string_view previous = tree_.spelling(*it);
            if (it->kind == TokenKind::Identifier || it->kind == TokenKind::Number ||
                it->kind == TokenKind::String || it->kind == TokenKind::Regex)
                return false;
            if (it->kind == TokenKind::Template &&
                !previous.empty() && previous.back() == '`') return false;
            if (previous == ")" || previous == "]" || previous == "}" ||
                previous == "++" || previous == "--") return false;
            if (it->kind == TokenKind::Keyword)
                return previous != "this" && previous != "super" && previous != "true" &&
                       previous != "false" && previous != "null";
            return true;
        }
        return true;
    }

    std::size_t punctuator_length() const {
        static const std::array<std::string_view, 57> punctuators = {
            ">>>=", "**=", "&&=", "||=", "?" "?=", "===", "!==", ">>>", "<<=", ">>=",
            "...", "=>", "==", "!=", "<=", ">=", "++", "--", "&&", "||", "??",
            "?.", "**", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
            "{", "}", "(", ")", "[", "]", ".", ";", ",", "<", ">", "+", "-", "*",
            "%", "&", "|", "^", "!", "~", "?", ":", "=", "/"
        };
        for (const std::string_view punctuator : punctuators)
            if (starts_with(punctuator)) return punctuator.size();
        return 1;
    }

    bool identifier_start() const {
        const unsigned char byte = peek();
        return std::isalpha(byte) || byte == '_' || byte == '$' || byte >= 0x80 ||
               (byte == '\\' && peek(1) == 'u');
    }

    bool identifier_part() const {
        return identifier_start() || std::isdigit(peek());
    }

    bool is_keyword(std::string_view word) const {
        static const std::array<std::string_view, 49> keywords = {
            "as", "async", "await", "break", "case", "catch", "class", "const",
            "continue", "debugger", "default", "delete", "do", "else", "export",
            "extends", "false", "finally", "for", "from", "function", "get", "if",
            "implements", "import", "in", "instanceof", "interface", "let", "new", "null",
            "of", "package", "private", "protected", "public", "return", "set", "static",
            "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void"
        };
        for (const std::string_view keyword : keywords)
            if (word == keyword) return true;
        return word == "while" || word == "with" || word == "yield";
    }

    unsigned char peek(std::size_t offset = 0) const {
        return at_ + offset < source().size()
            ? static_cast<unsigned char>(source()[at_ + offset]) : 0;
    }
    bool starts_with(std::string_view text) const {
        return source().compare(at_, text.size(), text) == 0;
    }
    std::string_view slice(std::size_t begin, std::size_t end) const {
        return std::string_view(source()).substr(begin, end - begin);
    }
    const std::string& source() const { return tree_.source_; }
    void advance() {
        const char byte = source()[at_++];
        if (byte == '\n') { ++line_; column_ = 1; }
        else ++column_;
    }
    void add(TokenKind kind, std::size_t begin, std::size_t end) {
        tree_.tokens_.push_back({kind, {begin, end}});
    }
    bool fail(std::size_t begin, std::size_t end, std::size_t line,
              std::size_t column, const char* message) {
        diagnostic_ = {{begin, end}, line, column, message};
        return false;
    }

    SyntaxTree& tree_;
    Diagnostic& diagnostic_;
    std::size_t at_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
};

ParseStatus parse_lossless(std::string source, SyntaxTree& output,
                           Diagnostic& diagnostic) {
    SyntaxTree candidate;
    candidate.source_ = std::move(source);
    Scanner scanner(candidate, diagnostic);
    if (scanner.scan() == ParseStatus::SyntaxError) return ParseStatus::SyntaxError;
    candidate.nodes_.push_back({NodeKind::Root, SemanticStatus::Opaque,
                                {0, candidate.source_.size()}, 0, 0,
                                candidate.tokens_.size()});
    struct Delimiter {
        char close;
        std::size_t node;
    };
    std::vector<Delimiter> delimiters;
    const auto position = [&](std::size_t offset) {
        std::size_t line = 1, column = 1;
        for (std::size_t i = 0; i < offset; ++i) {
            if (candidate.source_[i] == '\n') { ++line; column = 1; }
            else ++column;
        }
        return std::pair<std::size_t, std::size_t>{line, column};
    };
    const auto structural_error = [&](const Token& token, const char* message) {
        const auto [line, column] = position(token.range.begin);
        diagnostic = {token.range, line, column, message};
        return ParseStatus::SyntaxError;
    };
    for (std::size_t token_index = 0; token_index < candidate.tokens_.size();
         ++token_index) {
        const Token& token = candidate.tokens_[token_index];
        const std::string_view spelling = candidate.spelling(token);
        char actual_close = 0;
        if (token.kind == TokenKind::Punctuator && spelling.size() == 1 &&
            (spelling[0] == ')' || spelling[0] == ']' || spelling[0] == '}'))
            actual_close = spelling[0];
        else if (token.kind == TokenKind::Template && !spelling.empty() &&
                 spelling.front() == '}')
            actual_close = '}';
        if (actual_close) {
            if (delimiters.empty() || delimiters.back().close != actual_close)
                return structural_error(token, "unmatched closing delimiter");
            Node& node = candidate.nodes_[delimiters.back().node];
            node.range.end = token.range.end;
            node.last_token = token_index + 1;
            delimiters.pop_back();
        }
        char close = 0;
        if (token.kind == TokenKind::Punctuator) {
            if (spelling == "(") close = ')';
            else if (spelling == "[") close = ']';
            else if (spelling == "{") close = '}';
        } else if (token.kind == TokenKind::Template &&
                   spelling.size() >= 2 &&
                   spelling.substr(spelling.size() - 2) == "${") {
            close = '}';
        }
        if (close) {
            const std::size_t parent = delimiters.empty() ? 0 : delimiters.back().node;
            const std::size_t node = candidate.nodes_.size();
            candidate.nodes_.push_back({NodeKind::Delimited, SemanticStatus::Opaque,
                                        {token.range.begin, token.range.end}, parent,
                                        token_index, token_index + 1});
            delimiters.push_back({close, node});
        }
    }
    if (!delimiters.empty()) {
        const Node& node = candidate.nodes_[delimiters.back().node];
        return structural_error(candidate.tokens_[node.first_token],
                                "unclosed delimiter");
    }

    // Retain conservative expression islands without weakening the delimiter
    // barriers around them.  These are deliberately ranges rather than an
    // executable AST: consumers may reason about their ownership, while an
    // unsupported token keeps the enclosing construct opaque.
    const auto significant = [&](std::size_t token) {
        return candidate.tokens_[token].kind != TokenKind::Comment &&
               candidate.tokens_[token].kind != TokenKind::Hashbang;
    };
    const auto expression_keyword = [](std::string_view word) {
        static const std::array<std::string_view, 13> words = {
            "await", "delete", "false", "function", "new", "null", "super",
            "this", "true", "typeof", "undefined", "void", "yield"
        };
        for (const auto candidate : words)
            if (word == candidate) return true;
        return false;
    };
    const auto expression_punctuator = [](std::string_view punctuation) {
        static const std::array<std::string_view, 42> allowed = {
            "(", ")", "[", "]", "{", "}", ".", "?.", ",", ":", "...",
            "+", "-", "*", "/", "%", "**", "++", "--", "!", "~", "&&",
            "||", "??", "?", "=", "+=", "-=", "*=", "/=", "%=", "**=",
            "&&=", "||=", "?" "?=", "==", "!=", "===", "!==", "<", ">",
            "<="
        };
        if (punctuation == ">=" || punctuation == "=>" || punctuation == "&" ||
            punctuation == "|" || punctuation == "^" || punctuation == "<<" ||
            punctuation == ">>" || punctuation == ">>>" || punctuation == "&=" ||
            punctuation == "|=" || punctuation == "^=" || punctuation == "<<=" ||
            punctuation == ">>=" || punctuation == ">>>=") return true;
        for (const auto candidate : allowed)
            if (punctuation == candidate) return true;
        return false;
    };
    const auto expression_range = [&](std::size_t first, std::size_t last,
                                      std::size_t parent) {
        while (first < last && !significant(first)) ++first;
        while (first < last && !significant(last - 1)) --last;
        if (first == last) return;
        bool operand = false;
        for (std::size_t i = first; i < last; ++i) {
            const Token& token = candidate.tokens_[i];
            if (!significant(i)) continue;
            if (token.kind == TokenKind::Identifier || token.kind == TokenKind::Number ||
                token.kind == TokenKind::String || token.kind == TokenKind::Regex ||
                token.kind == TokenKind::Template) {
                operand = true;
                continue;
            }
            const std::string_view word = candidate.spelling(token);
            if (token.kind == TokenKind::Keyword && expression_keyword(word)) {
                operand = true;
                continue;
            }
            if (token.kind != TokenKind::Punctuator || !expression_punctuator(word))
                return;
        }
        if (!operand) return;
        const SourceRange range{candidate.tokens_[first].range.begin,
                                candidate.tokens_[last - 1].range.end};
        candidate.nodes_.push_back({NodeKind::Expression, SemanticStatus::Understood,
                                    range, parent, first, last});
    };

    // Initializers and return/throw operands are high-value safe islands.
    std::size_t boundary = 0;
    for (std::size_t i = 0; i <= candidate.tokens_.size(); ++i) {
        const bool end = i == candidate.tokens_.size();
        const std::string_view word = end ? std::string_view{} : candidate.spelling(candidate.tokens_[i]);
        if (end || word == ";") {
            std::size_t expression_begin = boundary;
            for (std::size_t j = boundary; j < i; ++j) {
                const std::string_view part = candidate.spelling(candidate.tokens_[j]);
                if (part == "=" || part == "return" || part == "throw" || part == "=>")
                    expression_begin = j + 1;
            }
            expression_range(expression_begin, i, 0);
            boundary = i + 1;
        }
    }
    // Parenthesized/bracketed contents are useful independently of their
    // enclosing statement and remain bounded by the already validated pairs.
    const std::size_t delimiter_count = candidate.nodes_.size();
    for (std::size_t i = 1; i < delimiter_count; ++i) {
        const Node& node = candidate.nodes_[i];
        if (node.kind != NodeKind::Delimited || node.last_token <= node.first_token + 1)
            continue;
        const std::string_view opener = candidate.spelling(candidate.tokens_[node.first_token]);
        if (opener == "(" || opener == "[")
            expression_range(node.first_token + 1, node.last_token - 1, i);
    }

    // Partition semicolon-terminated statements within the root and brace
    // containers. Parentheses (notably for-loop headers), brackets and
    // templates are intentionally excluded from statement ownership.
    std::vector<std::size_t> token_container(candidate.tokens_.size(), 0);
    std::vector<std::vector<std::size_t>> opens(candidate.tokens_.size());
    std::vector<std::vector<std::size_t>> closes(candidate.tokens_.size() + 1);
    for (std::size_t i = 1; i < delimiter_count; ++i) {
        const Node& node = candidate.nodes_[i];
        opens[node.first_token].push_back(i);
        closes[node.last_token].push_back(i);
    }
    std::vector<std::size_t> containers;
    for (std::size_t token = 0; token < candidate.tokens_.size(); ++token) {
        for (const auto node : closes[token]) {
            if (!containers.empty() && containers.back() == node)
                containers.pop_back();
        }
        for (const auto node : opens[token]) containers.push_back(node);
        token_container[token] = containers.empty() ? 0 : containers.back();
    }
    const auto statement_container = [&](std::size_t container) {
        if (container == 0) return true;
        const Node& node = candidate.nodes_[container];
        return candidate.spelling(candidate.tokens_[node.first_token]) == "{";
    };
    const auto add_statement = [&](std::size_t first, std::size_t last,
                                   std::size_t parent) {
        while (first < last && !significant(first)) ++first;
        while (first < last && !significant(last - 1)) --last;
        if (first == last) return;
        const std::string_view initial = candidate.spelling(candidate.tokens_[first]);
        if (initial == "class" || initial == "import" || initial == "export" ||
            initial == "case" || initial == "default") return;
        candidate.nodes_.push_back({NodeKind::Statement, SemanticStatus::Understood,
                                    {candidate.tokens_[first].range.begin,
                                     candidate.tokens_[last - 1].range.end},
                                    parent, first, last});
    };
    std::vector<std::size_t> statement_begin(delimiter_count, 0);
    for (std::size_t i = 1; i < delimiter_count; ++i)
        statement_begin[i] = candidate.nodes_[i].first_token + 1;
    for (std::size_t token = 0; token < candidate.tokens_.size(); ++token) {
        const std::size_t container = token_container[token];
        if (!statement_container(container)) continue;
        const std::string_view punctuation = candidate.spelling(candidate.tokens_[token]);
        if (punctuation != ";") continue;
        bool empty_control_body = false;
        if (token > 0 && candidate.spelling(candidate.tokens_[token - 1]) == ")") {
            for (const auto i : closes[token]) {
                const Node& header = candidate.nodes_[i];
                if (header.first_token == 0) continue;
                const std::string_view control = candidate.spelling(
                    candidate.tokens_[header.first_token - 1]);
                empty_control_body = control == "while" || control == "for" ||
                                     control == "if" || control == "with";
                if (empty_control_body) break;
            }
        }
        if (empty_control_body)
            candidate.nodes_.push_back({NodeKind::Statement, SemanticStatus::Understood,
                                        candidate.tokens_[token].range, container,
                                        token, token + 1});
        const std::size_t first = statement_begin[container];
        if (first == token && !empty_control_body) {
            // Empty statements are semantically meaningful, including loop
            // bodies such as `while (condition);`.
            candidate.nodes_.push_back({NodeKind::Statement, SemanticStatus::Understood,
                                        candidate.tokens_[token].range, container,
                                        token, token + 1});
        } else {
            add_statement(first, token + 1, container);
        }
        statement_begin[container] = token + 1;
    }
    // Braced blocks have exact boundaries even when their contents remain a
    // mixture of understood statements and opaque syntax.
    for (std::size_t i = 1; i < delimiter_count; ++i) {
        const Node& block = candidate.nodes_[i];
        if (candidate.spelling(candidate.tokens_[block.first_token]) != "{") continue;
        candidate.nodes_.push_back({NodeKind::Statement, SemanticStatus::Understood,
                                    block.range, block.parent, block.first_token,
                                    block.last_token});
    }

    const auto delimiter_opening_at = [&](std::size_t token,
                                          std::string_view spelling) {
        if (token >= opens.size()) return std::size_t{0};
        for (const auto node : opens[token])
            if (candidate.spelling(candidate.tokens_[token]) == spelling)
                return node;
        return std::size_t{0};
    };
    const auto next_significant = [&](std::size_t token) {
        while (token < candidate.tokens_.size() && !significant(token)) ++token;
        return token;
    };
    const auto previous_significant = [&](std::size_t token) {
        while (token > 0) {
            --token;
            if (significant(token)) return token;
        }
        return candidate.tokens_.size();
    };
    const auto add_function = [&](std::size_t begin, std::size_t end,
                                  std::size_t parameter_first,
                                  std::size_t parameter_last,
                                  SourceRange parameter_range,
                                  std::size_t parent) {
        if (begin >= end || end > candidate.tokens_.size()) return;
        const std::size_t function = candidate.nodes_.size();
        candidate.nodes_.push_back({NodeKind::Function, SemanticStatus::Understood,
                                    {candidate.tokens_[begin].range.begin,
                                     candidate.tokens_[end - 1].range.end},
                                    parent, begin, end});
        candidate.nodes_.push_back({NodeKind::Parameters, SemanticStatus::Understood,
                                    parameter_range, function, parameter_first,
                                    parameter_last});
    };
    // Ordinary functions: retain the whole header/body and parameter range.
    for (std::size_t token = 0; token < candidate.tokens_.size(); ++token) {
        if (candidate.spelling(candidate.tokens_[token]) != "function") continue;
        std::size_t parameter_token = next_significant(token + 1);
        if (parameter_token < candidate.tokens_.size() &&
            candidate.spelling(candidate.tokens_[parameter_token]) == "*")
            parameter_token = next_significant(parameter_token + 1);
        if (parameter_token < candidate.tokens_.size() &&
            candidate.tokens_[parameter_token].kind == TokenKind::Identifier)
            parameter_token = next_significant(parameter_token + 1);
        const std::size_t parameters = delimiter_opening_at(parameter_token, "(");
        if (!parameters) continue;
        const std::size_t body_token = next_significant(
            candidate.nodes_[parameters].last_token);
        const std::size_t body = delimiter_opening_at(body_token, "{");
        if (!body) continue;
        const Node parameter_region = candidate.nodes_[parameters];
        add_function(token, candidate.nodes_[body].last_token,
                     parameter_region.first_token, parameter_region.last_token,
                     parameter_region.range,
                     candidate.nodes_[body].parent);
    }
    // Arrows may use one identifier or a parenthesized parameter list. Braced
    // bodies are exactly bounded; expression bodies stop at their owning
    // comma/semicolon without crossing a delimiter container.
    for (std::size_t arrow = 0; arrow < candidate.tokens_.size(); ++arrow) {
        if (candidate.spelling(candidate.tokens_[arrow]) != "=>") continue;
        const std::size_t previous = previous_significant(arrow);
        if (previous == candidate.tokens_.size()) continue;
        std::size_t parameters = 0;
        std::size_t begin = previous;
        std::size_t parameter_first = previous, parameter_last = previous + 1;
        SourceRange parameter_range = candidate.tokens_[previous].range;
        if (candidate.spelling(candidate.tokens_[previous]) == ")") {
            for (const auto node : closes[previous + 1]) {
                if (candidate.spelling(
                        candidate.tokens_[candidate.nodes_[node].first_token]) == "(") {
                    parameters = node;
                    begin = candidate.nodes_[node].first_token;
                    parameter_first = candidate.nodes_[node].first_token;
                    parameter_last = candidate.nodes_[node].last_token;
                    parameter_range = candidate.nodes_[node].range;
                    break;
                }
            }
        } else if (candidate.tokens_[previous].kind == TokenKind::Identifier) {
            parameters = candidate.nodes_.size(); // non-zero presence marker
        }
        if (!parameters) continue;
        const std::size_t body_token = next_significant(arrow + 1);
        if (body_token >= candidate.tokens_.size()) continue;
        const std::size_t body = delimiter_opening_at(body_token, "{");
        std::size_t end = body ? candidate.nodes_[body].last_token : body_token + 1;
        if (!body) {
            const std::size_t container = token_container[body_token];
            while (end < candidate.tokens_.size() &&
                   token_container[end] == container) {
                const std::string_view separator = candidate.spelling(candidate.tokens_[end]);
                if (separator == ";" || separator == ",") break;
                ++end;
            }
        }
        add_function(begin, end, parameter_first, parameter_last, parameter_range,
                     token_container[begin]);
    }
    output = std::move(candidate);
    diagnostic = {};
    return ParseStatus::Success;
}

} // namespace jspp::syntax
