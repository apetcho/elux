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

/*
void Tokenizer::skip_comment(void){}
void Tokenizer::peek(i32 idx){}
void Tokenizer::advance(i32 count){}

Token Tokenizer::read_identifier(void){}
Token Tokenizer::read_integer_or_float(void){}
Token Tokenizer::read_string(void){}

bool Tokenizer::is_eos(void){}
bool Tokenizer::check_eos(i64 ptr){}
*/


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-