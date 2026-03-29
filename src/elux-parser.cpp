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
bool Lexer::is_numeric(const std::string& str, TokenKind& kind){
    struct Checker{
        bool check(const std::string& src){
            return (
                this->is_decimal(src) ||
                this->is_hex(src) ||
                this->is_oct(src) ||
                this->is_bin(src)
            );
        }

    private:
        bool is_decimal(const std::string& src){
            std::string data{"0123456789"};
            // 0x...
            if(src.length()>=2 && src[0]=='+' && std::isdigit(src[1])){
                return true;
            }
            if(src.length()>=2 && src[0]=='-' && std::isdigit(src[1])){
                return true;
            }
            return std::isdigit(src[0]);
        }

        bool is_hex(const std::string& src){
            std::string data{"0123456789ABCDEFabcdef"};
            // 0x...
            if(src.length()>=3 && src[0]=='0' && src[1]=='x' && data.find(src[2])!=std::string::npos){
                return true;
            }
            return false;
        }

        bool is_oct(const std::string& src){
            std::string data{"01234567"};
            // 0x...
            if(src.length()>=3 && src[0]=='0' && src[1]=='o' && data.find(src[2])!=std::string::npos){
                return true;
            }
            return false;
        }

        bool is_bin(const std::string& src){
            std::string data{"01"};
            // 0x...
            if(src.length()>=3 && src[0]=='0' && src[1]=='b' && data.find(src[2])!=std::string::npos){
                return true;
            }
            return false;
        }
    };
    

    if(Checker{}.check(str)){
        if(str[0]=='0' && str[1]=='x'){
            bool failed{false};
            int i=2;
            for(auto c: str.substr(2)){
                if(std::string{"0123456789ABCDEFabcdef"}.find(c)==std::string::npos){
                    failed = true;
                    break;
                }
            }
            if(failed){
                kind = TokenKind::INVALID;
                return false;
            }
            kind = TokenKind::INT;
            return true;
        }
        // -
        if(str[0]=='0' && str[1]=='o'){
            bool failed{false};
            int i=2;
            for(auto c: str.substr(2)){
                if(std::string{"01234567"}.find(c)==std::string::npos){
                    failed = true;
                    break;
                }
            }
            if(failed){
                kind = TokenKind::INVALID;
                return false;
            }
            kind = TokenKind::INT;
            return true;
        }
        // -
        if(str[0]=='0' && str[1]=='b'){
            bool failed{false};
            int i=2;
            for(auto c: str.substr(2)){
                if(std::string{"01"}.find(c)==std::string::npos){
                    failed = true;
                    break;
                }
            }
            if(failed){
                kind = TokenKind::INVALID;
                return false;
            }
            kind = TokenKind::INT;
            return true;
        }
        // -
        if((str[0]=='+' || str[0]=='-') && std::isdigit(str[1])){
            bool failed{false};
            bool fixed = (
                str.find('.')!=std::string::npos ||
                str.find('e')!=std::string::npos ||
                str.find('E')!=std::string::npos
            );
            
            std::string rest = str.substr(1);
            if(!fixed){//Float
                for(auto& c: rest){ c = std::tolower(c); }
                bool ok{true};
                auto pos = rest.find('.');
                if(pos!=std::string::npos){// mantissa
                    auto chunk = rest.substr(0, pos);
                    for(auto c: chunk){
                        if(!std::isdigit(c)){
                            ok = false;
                            break;
                        }
                    }
                    if(!ok){
                        kind = TokenKind::INVALID;
                        return false;
                    }
                    rest = rest.substr(pos+1);
                }
                if(rest.empty()){
                    kind = TokenKind::FLOAT;
                    return true;
                }

                // fractional & exponential part
                pos = rest.find('e');
                bool hasE{false};
                if(pos!=std::string::npos){ // fractional
                    hasE = true;
                    auto chunk = rest.substr(0, pos);
                    ok = true;
                    for(auto c: chunk){
                        if(!std::isdigit(c)){
                            ok = false;
                            break;
                        }
                    }
                    if(!ok){
                        kind = TokenKind::INVALID;
                        return false;
                    }
                    rest = rest.substr(pos+1);
                }
                if(hasE){
                    if(rest[0]=='-' || rest[0]=='+'){
                        rest = rest.substr(1);
                        ok = true;
                        for(auto c: rest){
                            if(!std::isdigit(c)){
                                ok = false;
                                break;
                            }
                        }
                        if(!ok){
                            kind = TokenKind::INVALID;
                            return false;
                        }
                    }else{
                        ok = true;
                        for(auto c: rest){
                            if(!std::isdigit(c)){
                                ok = false;
                                break;
                            }
                        }
                        if(!ok){
                            kind = TokenKind::INVALID;
                            return false;
                        }
                    }
                }
                kind = TokenKind::FLOAT;
                return true;
            }
            kind = TokenKind::INT;
            auto ok = true;
            for(auto c: rest){
                if(!std::isspace(c)){
                    ok = false;
                    break;
                }
            }
            if(!ok){
                kind = TokenKind::INVALID;
                return false;
            }
            return true;
        }else if(std::isdigit(str[0])){
            bool failed{false};
            bool fixed = (
                str.find('.')!=std::string::npos ||
                str.find('e')!=std::string::npos ||
                str.find('E')!=std::string::npos
            );
            
            std::string rest{str};
            if(!fixed){//Float
                for(auto& c: rest){ c = std::tolower(c); }
                bool ok{true};
                auto pos = rest.find('.');
                if(pos!=std::string::npos){// mantissa
                    auto chunk = rest.substr(0, pos);
                    for(auto c: chunk){
                        if(!std::isdigit(c)){
                            ok = false;
                            break;
                        }
                    }
                    if(!ok){
                        kind = TokenKind::INVALID;
                        return false;
                    }
                    rest = rest.substr(pos+1);
                }
                if(rest.empty()){
                    kind = TokenKind::FLOAT;
                    return true;
                }

                // fractional & exponential part
                pos = rest.find('e');
                bool hasE{false};
                if(pos!=std::string::npos){ // fractional
                    hasE = true;
                    auto chunk = rest.substr(0, pos);
                    ok = true;
                    for(auto c: chunk){
                        if(!std::isdigit(c)){
                            ok = false;
                            break;
                        }
                    }
                    if(!ok){
                        kind = TokenKind::INVALID;
                        return false;
                    }
                    rest = rest.substr(pos+1);
                }
                if(hasE){
                    if(rest[0]=='-' || rest[0]=='+'){
                        rest = rest.substr(1);
                        ok = true;
                        for(auto c: rest){
                            if(!std::isdigit(c)){
                                ok = false;
                                break;
                            }
                        }
                        if(!ok){
                            kind = TokenKind::INVALID;
                            return false;
                        }
                    }else{
                        ok = true;
                        for(auto c: rest){
                            if(!std::isdigit(c)){
                                ok = false;
                                break;
                            }
                        }
                        if(!ok){
                            kind = TokenKind::INVALID;
                            return false;
                        }
                    }
                }
                kind = TokenKind::FLOAT;
                return true;
            }
            kind = TokenKind::INT;
            auto ok = true;
            for(auto c: rest){
                if(!std::isspace(c)){
                    ok = false;
                    break;
                }
            }
            if(!ok){
                kind = TokenKind::INVALID;
                return false;
            }
            return true;
        }
        kind = TokenKind::SYMBOL;
        return true;
    }
}

// -*-
Token Lexer::next_token(void){
    Token result;
    skip();
    
    if(this->eof()){
        result.offset = this->m_offset;
        result.row = this->m_row;
        result.col = this->m_col;
        return result;
    }
    char c = this->peek();
    result.offset = this->m_offset;
    result.row = this->m_row;
    result.col = this->m_col;
    
    if(c=='('){
        result.kind = TokenKind::LPAREN;
        result.text = "(";
        this->advance();
        return result;
    }
    if(c==')'){
        result.kind = TokenKind::RPAREN;
        result.text = ")";
        this->advance();
        return result;
    }
    if(c=='\''){
        result.kind = TokenKind::QUOTE;
        result.text = "'";
        this->advance();
        return result;
    }
    if(c=='`'){
        result.kind = TokenKind::BACKQUOTE;
        result.text = "`";
        this->advance();
        return result;
    }
    if(c==','){
        if (m_pos + 1 < m_src.size() && this->peek_next()=='@') {
            // m_pos += 2;
            result.kind = TokenKind::COMMA_AT;
            result.text = ",@";
            this->advance();
            this->advance();
            return result;
        }
        result.kind = TokenKind::COMMA;
        result.text = ",";
        this->advance();
        return result;
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
        result.kind = TokenKind::STRING;
        result.text = oss.str();
        return result;
    }

    // -*-
    auto text = this->read_token();
    TokenKind kind;
    if(this->is_numeric(text, kind)){
        result.kind = kind;
        result.text = text;
        return result;
    }

    result.kind = kind;
    result.text = text;
    return result;

    // //! @todo
    // if(isdigit((unsigned char)c) || (c == '-' && m_pos+1 < m_src.size() && isdigit(this->peek_next()))){
    //     size_t start = m_pos;
    //     bool hasDot = false;
    //     if(this->peek()=='-'){
    //         this->advance();
    //         // pos++;
    //     }
    //     while(!this->eof() && (isdigit(this->peek()) || this->peek()=='.')) {
    //         if (this->peek()=='.'){ hasDot = true; }
    //         this->advance();
    //     }
    //     std::string num = m_src.substr(start, m_pos - start);
    //     return {hasDot ? TokenKind::FLOAT : TokenKind::INT, num};
    // }
    // // symbol

    // size_t start = m_pos;
    // while(!this->eof() && !isspace(this->peek()) &&
    //     this->peek() != '(' && this->peek() != ')' && this->peek() != '\'' &&
    //     this->peek() != '`' && this->peek() != ','
    // ){
    //     this->advance();
    // }
    // return {TokenKind::SYMBOL, m_src.substr(start, m_pos - start)};
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
    auto c = this->peek();
    if(c=='\n'){
        this->m_row += 1;
        this->m_col = 1;
        ++this->m_offset;
    }else{
        this->m_col += 1;
    }
}

// -*-
Parser::Parser(const std::string& s) : m_lexer(s) {
    m_token = m_lexer.next_token();
}

// -*-
void Parser::advance(void){ m_token = m_lexer.next_token(); }

// -*-
bool Parser::match(TokenKind kind){
    if (m_token.kind == kind){
        advance();
        return true;
    }
    return false;
}

// -*-
Expr Parser::parse_expr(){
    switch (m_token.kind){
    case TokenKind::INVALID:{
            std::stringstream ss;
            ss << "invalid token" << std::quoted(m_token.text) << " found at ";
            ss << "row " << m_token.row << " and column " << m_token.col << ".\n";
            auto start = this->m_token.offset;
            auto len = this->m_token.col;
            auto text = this->m_lexer.get_line(start);
            auto line = std::string((len-1), ' ');
            line += "^\n";
            ss << text << "\n";
            ss << line;
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        break;
    case TokenKind::LPAREN:
        return parse_list();
    case TokenKind::QUOTE: {
        advance();
        return this->make_list_expr({
            this->make_symbol_expr("quote"),
            this->parse_expr(),
        });
    }
    case TokenKind::BACKQUOTE: {
        advance();
        return this->make_list_expr({
            this->make_symbol_expr("quasiquote"),
            this->parse_expr(),
        });
    }
    case TokenKind::COMMA: {
        advance();
        return this->make_list_expr({
            this->make_symbol_expr("unquote"),
            this->parse_expr(),
        });
    }
    case TokenKind::COMMA_AT: {
        advance();
        return this->make_list_expr({
            this->make_symbol_expr("unquote-splicing"),
            this->parse_expr(),
        });
    }
    case TokenKind::STRING:{
        auto text = this->m_token.text;
        advance();
        return this->make_literal_expr(text);
    }
    case TokenKind::INT: {
        auto text = this->m_token.text;
        i64 num{};
        size_t pos{};
        if(text.length() > 2 && text[0]=='0' && text[0]=='x'){
            num = std::stoll(text, &pos, 16);
        }else if(text.length() > 2 && text[0]=='0' && text[0]=='o'){
            num = std::stoll(text, &pos, 8);
        }else if(text.length() > 2 && text[0]=='0' && text[0]=='b'){
            num = std::stoll(text, &pos, 2);
        }else{
            num = std::stoll(text, &pos);
        }
        advance();

        return this->make_literal_expr(num);
    }
    case TokenKind::FLOAT:{
        auto text = this->m_token.text;
        auto num = std::stod(text);
        advance();
        return this->make_literal_expr(num);
    }
    case TokenKind::SYMBOL:{
        std::string text = m_token.text;
        advance();
        if(text=="nil"){ return this->make_literal_expr(); }
        if(text=="true"){ return this->make_literal_expr(true); }
        if(text=="false"){ return this->make_literal_expr(false); }
        return this->make_symbol_expr(text);
    }
    default:
        throw ELuxError(ELuxError::SyntaxError, "Unexpected token in parseExpr");
    }
}

// -*-
Expr Parser::parse_list() {
    if(!match(TokenKind::LPAREN)){
        throw ELuxError(ELuxError::SyntaxError, "Expected '('");
    }
    Vec<Expr> exprs{};
    while(m_token.kind != TokenKind::RPAREN && m_token.kind != TokenKind::END){
        exprs.push_back(this->parse_expr());
    }
    if(!match(TokenKind::RPAREN)){
        throw ELuxError(ELuxError::SyntaxError, "Expected ')'");
    }
    return this->make_list_expr(std::move(exprs));
}

// -*-
Vec<Expr> Parser::parse(){
    Vec<Expr> exprs;
    while(m_token.kind != TokenKind::END){
        exprs.push_back(parse_expr());
    }
    return exprs;
}

// -*-
Expr Parser::make_symbol_expr(const std::string& text){
    auto expr = std::make_shared<SymbolExpr>();
    expr->name = Symbol(text);
    return std::move(expr);
}

// -*-
Expr Parser::make_literal_expr(void){
    auto expr = std::make_shared<LiteralExpr>();
    expr->value = std::make_shared<Nil>();
    return std::move(expr);
}
 
// -*-
Expr Parser::make_literal_expr(bool val){
    auto expr = std::make_shared<LiteralExpr>();
    expr->value = std::make_shared<Bool>(val);
    return std::move(expr);
}

// -*-
Expr Parser::make_literal_expr(i64 val){
    auto expr = std::make_shared<LiteralExpr>();
    expr->value = std::make_shared<Number>(val);
    return std::move(expr);
}

// -*-
Expr Parser::make_literal_expr(f64 val){
    auto expr = std::make_shared<LiteralExpr>();
    expr->value = std::make_shared<Number>(val);
    return std::move(expr);
}

// -*-
Expr Parser::make_literal_expr(const std::string& text){
    auto expr = std::make_shared<LiteralExpr>();
    expr->value = std::make_shared<String>(text);
    return std::move(expr);
}

// -*-
Expr Parser::make_list_expr(Vec<Expr>&& expr){
    //! @todo
    return nullptr;
}

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-