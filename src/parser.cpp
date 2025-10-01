#include "lynx.hpp"


// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -

// --------------
// -*- Parser -*-
// --------------
Parser::Parser(std::istringstream&& stream) noexcept
: m_tokenizer{Tokenizer(std::move(stream))}
{
    this->m_row = this->m_tokenizer.m_row;
    this->m_col = this->m_tokenizer.m_col;
}

// -*-
Parser::Parser(std::ifstream&& stream) noexcept
: m_tokenizer{Tokenizer(std::move(stream))}
{
    this->m_row = this->m_tokenizer.m_row;
    this->m_col = this->m_tokenizer.m_col;
}

// -*-
Parser::Parser(Parser&& parser) noexcept
: m_tokenizer{std::move(parser.m_tokenizer)}
, m_row{std::move(parser.m_row)}
, m_col{std::move(parser.m_col)}
{}

/*
Parser& Parser::operator=(Parser&& parser) noexcept{}

Result Parser::parse(void){}

Result Parser::parse_atom(void){}
Result Parser::parse_list(void){}
bool Parser::match(const Str& word, const Token& token){}
bool Parser::match(TokenKind expectedKind, const Token& token){}
void Parser::expect(TokenKind expectedKind, const Token& token){}

*/

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-