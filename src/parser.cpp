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

// -*-
Parser& Parser::operator=(Parser&& parser) noexcept{
    if(this != &parser){
        this->m_tokenizer = std::move(parser.m_tokenizer);
        this->m_row = std::move(parser.m_row);
        this->m_col = std::move(parser.m_col);
    }

    return *this;
}

// -*-
Result Parser::parse(void){
    auto token = this->next_token();
    if(token.kind==TokenKind::Invalid){
        Error err;
        std::stringstream ss;
        ss << "unexpected token found while parsing: `" << token.lexeme << "' ";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }else if(token.kind==TokenKind::Eof){
        return Result(share()); // nil
    }else if(token.kind!=TokenKind::LParen){
        return this->parse_atom(token);
    }else if(token.kind==TokenKind::LParen){
        Vec<Self> vec{};
        while(token.kind!=TokenKind::RParen && token.kind!=TokenKind::Eof){
            auto ans = this->parse();
            if(ans.is_ok()){
                vec.push_back(ans.ok());
            }else{
                return Result(std::move(ans.err()));
            }
        }
        if(token.kind==TokenKind::Eof){
            std::stringstream ss;
            ss << "malformed s-expression. Missing ')' at ";
            ss << "row: " << this->m_row << ", column: " << this->m_col;
            Error error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(error));
        }
        // ')' has been already read.
        return Result(share(vec));
    }

    Error err("unexpected error occured.");
    return Result(std::move(err));
}

// -*-
Result Parser::parse_atom(const Token& token){
    if(token.kind==TokenKind::INTEGER){
        return Result(share(std::stoll(token.lexeme)));
    }else if(token.kind==TokenKind::FLOAT){
        return Result(share(std::stod(token.lexeme)));
    }else if(token.kind==TokenKind::STRING){
        return Result(share(token.lexeme));
    }
    return Result(share(token.lexeme.c_str()));
}

// -*-
Result Parser::parse_list(void){
    Vec<Self> vec{};
    auto token = this->next_token();
    while(token.kind!=TokenKind::RParen){
        if(token.kind==TokenKind::Eof){
            std::stringstream ss;
            ss << "malformed s-expression. Missing ')' at ";
            ss << "row: " << this->m_row << ", column: " << this->m_col;
            Error error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(error));
        }
        if(this->is_atom(token)){
            auto ans = this->parse_atom(token);
            if(ans.is_ok()){
                vec.push_back(ans.ok());
            }else{
                return Result(std::move(ans.err()));
            }
        }else if(token.kind==TokenKind::LParen){
            auto ans = this->parse_list();
            if(ans.is_ok()){
                vec.push_back(ans.ok());
            }else{
                return Result(std::move(ans.err()));
            }
        }
        token = this->next_token();
    }
    // ')' has been already read.
    return Result(share(vec));
}

// -*-
bool Parser::is_atom(const Token& token){
    return (
        token.kind==TokenKind::Ident ||
        token.kind==TokenKind::FLOAT ||
        token.kind==TokenKind::INTEGER ||
        token.kind==TokenKind::STRING ||
        token.kind==TokenKind::False ||
        token.kind==TokenKind::True ||
        token.kind==TokenKind::Nil
    );
}

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-