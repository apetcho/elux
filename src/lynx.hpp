#ifndef LYNX_HPP
#define LYNX_HPP

#include<filesystem>
#include<iostream>
#include<cstdint>

#define LYNX_KEYWORDS()                             \
    LYNX_DEF(Cond, "cond")                          \
    LYNX_DEF(For, "for")                            \
    LYNX_DEF(Fun, "fun")                            \
    LYNX_DEF(If, "if")                              \
    LYNX_DEF(Import, "import")                      \
    LYNX_DEF(Lambda, "lambda")                      \
    LYNX_DEF(Let, "let")                            \
    LYNX_DEF(Macro, "macro")                        \
    LYNX_DEF(Progn, "progn")                        \
    LYNX_DEF(Quasiquote, "quasiquote") /* ` */      \
    LYNX_DEF(Quote, "quote") /* ' */                \
    LYNX_DEF(Unquote, "unquote") /* , */            \
    LYNX_DEF(Var, "var")                            \
    LYNX_DEF(While, "while")


#define LYNX_TOKENS()               \
    LYNX_DEF(Invalid, "INVALID")    \
    LYNX_DEF(Eof, "EOF")            \
    LYNX_DEF(Plus, "+")             \
    LYNX_DEF(Minus, "-")            \
    LYNX_DEF(Div, "/")              \
    LYNX_DEF(Mul, "*")              \
    LYNX_DEF(Mod, "%")              \
    LYNX_DEF(Equal, "=")            \
    LYNX_DEF(And, "and")            \
    LYNX_DEF(Or, "or")              \
    LYNX_DEF(Not, "not")            \
    LYNX_DEF(Less, "<")             \
    LYNX_DEF(LessEq, "<=")          \
    LYNX_DEF(Greater, ">")          \
    LYNX_DEF(GreaterEq, ">=")       \
    LYNX_DEF(NotEq, "!=")           \
    LYNX_DEF(Nil, "nil")            \
    LYNX_DEF(True, "true")          \
    LYNX_DEF(False, "false")        \
    LYNX_KEYWORDS()


// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -
// Forward class
class Error;
class Env;
class Result;

// Type aliases

/*
class Object{};
class Bool final: public Object{};
class Number final: public Object{};
class Symbol final: public Object{};
class String final: public Object{};
class Pair final: public Object{};
class List final: public Object{};
class HashSet final: public Object{};
class HashMap final: public Object{};
class Array final: public Object{};
class Builtin final: public Object{};
class Closure final: public Object{};
class Macro final: public Object{};

struct Token final{};
class Tokenizer final{};
class Parser final{};

class Lynx final{};

*/


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-

#endif