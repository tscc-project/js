#include <jspp/syntax.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: syntax-inventory FILE\n";
        return 2;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "cannot read " << argv[1] << '\n';
        return 2;
    }
    std::string source((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());
    jspp::syntax::SyntaxTree tree;
    jspp::syntax::Diagnostic diagnostic;
    const auto status = jspp::syntax::parse_lossless(std::move(source), tree, diagnostic);
    if (status == jspp::syntax::ParseStatus::SyntaxError) {
        std::cout << "error\t" << diagnostic.line << '\t' << diagnostic.column
                  << '\t' << diagnostic.message << '\n';
        return 1;
    }
    std::size_t opaque = 0, delimited = 0, expressions = 0, statements = 0,
                functions = 0, parameters = 0, classes = 0, imports = 0,
                exports = 0,
                identifiers = 0, regexes = 0,
                templates = 0;
    for (const auto& node : tree.nodes()) {
        opaque += node.semantics == jspp::syntax::SemanticStatus::Opaque;
        delimited += node.kind == jspp::syntax::NodeKind::Delimited;
        expressions += node.kind == jspp::syntax::NodeKind::Expression;
        statements += node.kind == jspp::syntax::NodeKind::Statement;
        functions += node.kind == jspp::syntax::NodeKind::Function;
        parameters += node.kind == jspp::syntax::NodeKind::Parameters;
        classes += node.kind == jspp::syntax::NodeKind::Class;
        imports += node.kind == jspp::syntax::NodeKind::ImportDeclaration;
        exports += node.kind == jspp::syntax::NodeKind::ExportDeclaration;
    }
    for (const auto& token : tree.tokens()) {
        identifiers += token.kind == jspp::syntax::TokenKind::Identifier;
        regexes += token.kind == jspp::syntax::TokenKind::Regex;
        templates += token.kind == jspp::syntax::TokenKind::Template;
    }
    std::cout << "ok\t" << tree.source().size() << '\t' << tree.tokens().size()
              << '\t' << tree.nodes().size() << '\t' << opaque << '\t'
              << delimited << '\t' << identifiers << '\t' << regexes << '\t'
              << templates << '\t' << expressions << '\t' << statements << '\t'
              << functions << '\t' << parameters << '\t' << classes << '\t'
              << imports << '\t' << exports << '\n';
}
