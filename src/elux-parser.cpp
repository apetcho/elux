/*
MIT License

Copyright (c) 2026 Eyram K. Apetcho

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "elux.hpp"


// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace ekasoft::klx{
// -
// -*-
Lexer::Lexer(std::string s)
: m_src(std::move(s)) {}

void Lexer::skip() {
    while (!this->eof() && isspace(this->peek())){
        this->advance();
    }
    if(this->eof()){ return; }
    if(this->peek()==';'){ // skip the comment
        while (!this->eof() && isspace(this->peek())!='\n'){
            this->advance();
        }   
    }
}

// void Lexer::skip_comment(void){
//     if(this->peek()==';'){
//         while(!this->eof() && this->peek() != '\n'){ this->advance(); }
//     }
// }

bool Lexer::startsWith(const std::string& s) {
    return m_src.compare(m_pos, s.size(), s) == 0;
}

// -*-
std::string Lexer::read_token(void){
    auto reserved_char = [](char c){
        static std::string specials = "()`',";
        return (specials.find(c)!=std::string::npos);
    };

    size_t start = m_pos;
    auto c = this->peek();
    while(!this->eof() && !isspace(c) && !reserved_char(c)){
        this->advance();
        c = this->peek();
    }
    return m_src.substr(start, m_pos - start);
}

// -*-
Token Lexer::next(){
    skip();
    
    if(this->eof()){ return {TokenKind::END, ""}; }
    char c = this->peek();

    if(c=='('){
        this->advance();
        return {TokenKind::LPAREN, "("};
    }
    if(c==')'){
        this->advance();
        return {TokenKind::RPAREN, ")"};
    }
    if(c=='\''){
        this->advance();
        return {TokenKind::QUOTE, "'"};
    }
    if(c=='`'){
        this->advance();
        return {TokenKind::BACKQUOTE, "`"};
    }
    if(c==','){
        if (m_pos + 1 < m_src.size() && this->peek_next()=='@') {
            // m_pos += 2;
            this->advance();
            this->advance();
            return {TokenKind::COMMA_AT, ",@"};
        }
        this->advance();
        return {TokenKind::COMMA, ","};
    }
    if(c=='"'){
        this->advance();
        std::ostringstream oss;
        while(!this->eof() && this->peek() != '"') {
            if(this->peek()=='\\' && m_pos + 1 < m_src.size()){
                //char n = m_src[++m_pos];
                char n = this->peek_next();
                this->advance();
                if(n=='n'){ oss << '\n'; }
                if(n=='t'){ oss << '\t'; }
                if(n=='f'){ oss << '\f'; }
                if(n=='b'){ oss << '\b'; }
                if(n=='"'){ oss << "\""; }
                else{ oss << n; }
            }else{
                // oss << src[pos++];
                oss << this->peek();
            }
            this->advance();
        }
        if(this->peek() == '"'){
            this->advance();
            //pos++;
        }
        return {TokenKind::STRING, oss.str()};
    }
    if(isdigit((unsigned char)c) || (c == '-' && m_pos+1 < m_src.size() && isdigit(this->peek_next()))){
        size_t start = m_pos;
        bool hasDot = false;
        if(this->peek()=='-'){
            this->advance();
            // pos++;
        }
        while(!this->eof() && (isdigit(this->peek()) || this->peek()=='.')) {
            if (this->peek()=='.'){ hasDot = true; }
            this->advance();
        }
        std::string num = m_src.substr(start, m_pos - start);
        return {hasDot ? TokenKind::FLOAT : TokenKind::INT, num};
    }
    // symbol
    size_t start = m_pos;
    while(!this->eof() && !isspace(this->peek()) &&
        this->peek() != '(' && this->peek() != ')' && this->peek() != '\'' &&
        this->peek() != '`' && this->peek() != ','
    ){
        this->advance();
    }
    return {TokenKind::SYMBOL, m_src.substr(start, m_pos - start)};
}

// -*-
bool Lexer::eof(void) const{
    if(this->m_pos >= this->m_src.length()){ return true; }
    return false;
}

// -*-
char Lexer::peek(void) const{
    if(this->eof()){ return EOF; }
    return this->m_src[this->m_pos];
}

// -*-
char Lexer::peek_next(void) const{
    if(this->m_pos + 1 >= this->m_src.length()){
        return EOF;
    }

    return this->m_src[this->m_pos+1];
}

// -*-
void Lexer::advance(void){
    this->m_pos++;
}

// -*-
Parser::Parser(const std::string& s) : m_lexer(s) {
    m_token = m_lexer.next();
}

// -*-
void Parser::advance() { m_token = m_lexer.next(); }

// -*-
bool Parser::match(TokenKind kind){
    if (m_token.kind == kind){
        advance();
        return true;
    }
    return false;
}

// -*-
Expr Parser::parse_expr() {
    switch (m_token.kind) {
    case TokenKind::LPAREN: return parse_list();
    case TokenKind::QUOTE: {
        advance();
        auto e = parse_expr();
        auto list = std::make_shared<ListExpr>();
        list->elements.push_back(std::make_shared<SymbolExpr>("quote"));
        list->elements.push_back(e);
        return list;
    }
    case TokenKind::BACKQUOTE: {
        advance();
        auto e = parse_expr();
        auto list = std::make_shared<ListExpr>();
        list->elements.push_back(std::make_shared<SymbolExpr>("quasiquote"));
        list->elements.push_back(e);
        return list;
    }
    case TokenKind::COMMA: {
        advance();
        auto e = parse_expr();
        auto list = std::make_shared<ListExpr>();
        list->elements.push_back(std::make_shared<SymbolExpr>("unquote"));
        list->elements.push_back(e);
        return list;
    }
    case TokenKind::COMMA_AT: {
        advance();
        auto e = parse_expr();
        auto list = std::make_shared<ListExpr>();
        list->elements.push_back(std::make_shared<SymbolExpr>("unquote-splicing"));
        list->elements.push_back(e);
        return list;
    }
    case TokenKind::STRING: {
        auto v = ELux::share(m_token.text);
        advance();
        return std::make_shared<LiteralExpr>(v);
    }
    case TokenKind::INT: {
        i64 i = std::stoll(m_token.text);
        auto v = ELux::share(i);
        advance();
        return std::make_shared<LiteralExpr>(v);
    }
    case TokenKind::FLOAT: {
        double d = std::stod(m_token.text);
        auto v = ELux::share(d);
        advance();
        return std::make_shared<LiteralExpr>(v);
    }
    case TokenKind::SYMBOL: {
        std::string s = m_token.text;
        advance();
        if(s=="nil"){ return std::make_shared<LiteralExpr>(ELux::share()); }
        if(s=="t"){ return std::make_shared<LiteralExpr>(ELux::share(true)); }
        return std::make_shared<SymbolExpr>(s);
    }
    default:
        throw std::runtime_error("Unexpected token in parseExpr");
    }
}

// -*-
Expr Parser::parse_list() {
    if(!match(TokenKind::LPAREN)){ throw std::runtime_error("Expected '('"); }
    auto list = std::make_shared<ListExpr>();
    while(m_token.kind != TokenKind::RPAREN && m_token.kind != TokenKind::END) {
        list->elements.push_back(parse_expr());
    }
    if(!match(TokenKind::RPAREN)){
        throw std::runtime_error("Expected ')'");
    }
    return list;
}

// -*-
Vec<Expr> Parser::parse(){
    Vec<Expr> exprs;
    while(m_token.kind != TokenKind::END){
        exprs.push_back(parse_expr());
    }
    return exprs;
}

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-