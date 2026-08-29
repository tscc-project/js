#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
namespace jspp {
struct Diagnostic { std::size_t offset=0,line=1,column=1;std::string message; };
enum class TokenKind { End,Identifier,Number,String,Keyword,Punct };
struct Token { TokenKind kind=TokenKind::End;std::string text;std::size_t offset=0,line=1,column=1; };
enum class ExprKind { Number,String,Boolean,Null,Undefined,Identifier,Unary,Binary };
struct Expr { ExprKind kind=ExprKind::Undefined;std::string text;double number=0;std::unique_ptr<Expr>left,right; };
enum class StatementKind { Declaration,Expression };
struct Statement { StatementKind kind=StatementKind::Expression;bool constant=false;std::string name;std::unique_ptr<Expr>expression; };
struct Program { std::vector<Statement> statements; };
class Lexer { public: explicit Lexer(std::string source):source_(std::move(source)){}bool lex(std::vector<Token>&,Diagnostic&);private:std::string source_; };
class Parser { public:Parser(const std::vector<Token>&t,Diagnostic&d):tokens_(t),diagnostic_(d){}bool parse(Program&);private:const std::vector<Token>&tokens_;Diagnostic&diagnostic_;std::size_t at_=0;std::unique_ptr<Expr>expression(int=0);std::unique_ptr<Expr>prefix();bool statement(Statement&);bool fail(const Token&,const std::string&); };
bool parse_source(const std::string&,Program&,Diagnostic&);
std::string format_diagnostic(const Diagnostic&);
}
