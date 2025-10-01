#include "lynx.hpp"
#include<cctype>

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -*-
// -*-------------*-
// -*- Tokenizer -*-
// -*-------------*-
Tokenizer::Tokenizer(std::istringstream&& stream) noexcept{
    this->m_kind = Kind::STR;
    this->m_sstream = std::move(stream);
    this->m_row = 1;
    this->m_col = 0;
}

// -*-
Tokenizer::Tokenizer(std::ifstream&& stream) noexcept{
    this->m_kind = Kind::FILE;
    this->m_fstream = std::move(stream);
    this->m_row = 1;
    this->m_col = 0;
}

// -*-
Tokenizer::Tokenizer(Tokenizer&& tokenizer) noexcept
: m_kind{std::move(tokenizer.m_kind)}
, m_sstream{std::move(tokenizer.m_sstream)}
, m_fstream{std::move(tokenizer.m_fstream)}
, m_row{std::move(tokenizer.m_row)}
, m_col{std::move(tokenizer.m_col)}
{}

// -*-
Tokenizer& Tokenizer::operator=(Tokenizer&& tokenizer) noexcept{
    if(this != &tokenizer){
        this->m_kind = std::move(tokenizer.m_kind);
        this->m_sstream = std::move(tokenizer.m_sstream);
        this->m_fstream = std::move(tokenizer.m_fstream);
        this->m_row = std::move(tokenizer.m_row);
        this->m_col = std::move(tokenizer.m_col);
    }
    return *this;
}

// -*-
Token Tokenizer::token(void){
    this->skip_whitespace();
    this->skip_comment();
    auto c = this->peek();
    if(c==EOF){
        return Token(TokenKind::Eof, "", -1, -1);
    }else if(c=='('){
        Token token;
        token.kind = TokenKind::LParen;
        token.row = this->m_row;
        token.col = this->m_col;
        token.lexeme = "(";
        this->advance();
        return token;
    }else if(c==')'){
        Token token;
        token.kind = TokenKind::RParen;
        token.row = this->m_row;
        token.col = this->m_col;
        token.lexeme = ")";
        this->advance();
        return token;
    }else if(this->is_syntax_quote(c)){
        return this->read_syntax_quote();
    }else if(c=='"'){
        return this->read_string();
    }else if(this->next_is_number(c)){
        return this->read_integer_or_float();
    }

    return this->read_identifier();
}

// -*-
TokenKind Tokenizer::token_kind_by_word(const Str& word){
    std::map<Str, TokenKind> _my_words_ = {
#define LYNX_DEF(tok, w)  {w, TokenKind::tok},
        LYNX_KEYWORDS()
#undef LYNX_DEF
    };
    _my_words_["true"] = TokenKind::True;
    _my_words_["false"] = TokenKind::False;
    _my_words_["nil"] = TokenKind::Nil;

    auto entry = _my_words_.find(word);
    if(entry != _my_words_.end()){
        return entry->second;
    }

    return TokenKind::Invalid;
}

// -*-
bool Tokenizer::is_symbol_char(i32 c){
    static const Str sym = ":_-+%*/!#$&=?`',@";
    auto pos = sym.find(c);
    return std::isalnum(c) || pos != Str::npos;
}

// -*-
bool Tokenizer::is_syntax_quote(i32 c){
    static const Str sym = ",`'";
    return sym.find(c) != Str::npos;
}

// -*-
void Tokenizer::skip_whitespace(void){
    while(std::isspace(this->peek()) && !this->is_eos()){
        this->advance();
    }
}

// -*-
void Tokenizer::skip_comment(void){
    auto c = this->peek();
    if(c==';'){
        while(c!='\n' && !this->is_eos()){
            this->advance();
        }
        if(c == '\n'){
            this->advance();
        }
    }
}

// -*-
i32 Tokenizer::peek(i32 idx){
    i32 c{};
    if(this->is_string_stream()){
        if(idx==0){
            c = this->m_sstream.peek();
        }else{
            auto pos = this->m_sstream.tellg();
            for(int i=0; i < idx; i++){
                this->advance();
                if(this->m_sstream.eof()){
                    return EOF;
                }
            }
            c = this->m_sstream.peek();
            this->m_sstream.seekg(pos, std::ios_base::beg);
        }
    }else{
        if(idx==0){
            c = this->m_fstream.peek();
        }else{
            auto pos = this->m_fstream.tellg();
            for(int i=0; i < idx; i++){
                this->advance();
                if(this->m_fstream.eof()){
                    return EOF;
                }
            }
            c = this->m_fstream.peek();
            this->m_fstream.seekg(pos, std::ios_base::beg);
        }
    }

    return c;
}

// -*-
void Tokenizer::advance(i32 count){
    i32 c{};
    if(this->is_string_stream()){
        for(int i=0; i < count; i++){
            if(this->m_sstream.eof()){
                break;
            }
            this->m_sstream.seekg(1, std::ios_base::cur);
            this->update_position();
        }
    }else{
        for(int i=0; i < count; i++){
            if(this->m_fstream.eof()){
                break;
            }
            this->m_fstream.seekg(1, std::ios_base::cur);
            this->update_position();
        }
    }
}

// -*-
void Tokenizer::update_position(void){
    auto c = this->peek();
    if(c == '\n'){
        this->m_row += 1;
        this->m_col = 1;
    }else{
        this->m_col += 1;
    }
}

// -*-
Token Tokenizer::read_identifier(void){
    auto row = this->m_row;
    auto col = this->m_col;
    Str ident{};
    auto c = this->peek();
    while(this->is_symbol_char(c)){
        ident += static_cast<char>(c);
        this->advance();
    }
    TokenKind kind;
    if(Lynx::is_reserved_word(ident)){
        kind = Tokenizer::token_kind_by_word(ident);
        if(kind == TokenKind::Invalid){
            kind = TokenKind::Ident;
        }
    }else{
        kind = TokenKind::Ident;
    }

    return Token(kind, ident, row, col);
}

// -*-
Token Tokenizer::read_integer_or_float(void){
    auto row = this->m_row;
    auto col = this->m_col;
    auto c = this->peek();
    auto isNum = (
        std::isdigit(c) || 
        (c=='-' && std::isdigit(this->peek(1))) ||
        (c=='+' && std::isdigit(this->peek(1)))
    );
    Token result;
    auto tok = this->read_identifier();
    result.row = row;
    result.col = col;
    result.lexeme = tok.lexeme;
    if(isNum){
        auto lexme = tok.lexeme;
        size_t len{};
        bool _floating_{false};
        _floating_ = (
            lexme.find(".") || lexme.find("e") || lexme.find("E")
        );
        if(_floating_){
            [[maybe_unused]] auto num = std::stod(lexme, &len);
            if(len==lexme.length()){
                result.kind = TokenKind::FLOAT;
            }
        }else{
            size_t len{};
            [[maybe_unused]] auto num = std::stoll(lexme, &len);
            if(len==lexme.length()){
                result.kind = TokenKind::INTEGER;
            }
        }
    }else{
        result.kind = TokenKind::Ident;
    }

    return result;
}

// -*-
Token Tokenizer::read_string(void){
    Token result{};
    result.row = this->m_row;
    result.col = this->m_col;
    this->advance(); // skip the opening double-quote
    Str text{};
    auto c = this->peek();
    text += static_cast<char>(c);
    while(c != '"' && !this->is_eos()){
        //! @note: maybe we need to handle escape-characters appropriately here
        this->advance();
        c = this->peek();
        text += static_cast<char>(c);
    }
    this->advance(); // skip the closing double-quote

    return result;
}

/*

bool Tokenizer::is_eos(void){}
*/


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-