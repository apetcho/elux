#include "lynx.hpp"

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

/*
Tokenizer::Tokenizer(Tokenizer&& tokenizer) noexcept{}
Tokenizer& Tokenizer::operator=(Tokenizer&& tokenizer) noexcept{}

Token Tokenizer::token(void){}

bool Tokenizer::is_symbol_char(i32 c){}
bool Tokenizer::is_syntax_quote(i32 c){}
void Tokenizer::skip_whitespace(void){}
void Tokenizer::peek(i32 idx){}
void Tokenizer::advance(i32 count){}

Token Tokenizer::read_symbol(void){}
Token Tokenizer::read_integer_or_float(void){}
Token Tokenizer::read_string(void){}

bool Tokenizer::is_eos(void){}
bool Tokenizer::check_eos(i64 ptr){}
*/


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-