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
#ifndef LYNX_HPP
#define LYNX_HPP

#include<initializer_list>
#include<unordered_map>
#include<unordered_set>
#include<filesystem>
#include<functional>
#include<stdexcept>
#include<cstdint>
#include<sstream>
#include<fstream>
#include<variant>
#include<memory>
#include<vector>
#include<string>
#include<list>
#include<map>
#include<set>


// -*-
namespace fs = std::filesystem;

// -*----------------------------------------------------------------*-
// -*- begin::namespace::ekasoft::klx                               -*-
// -*----------------------------------------------------------------*-
namespace ekasoft::klx{
// -


// =========================
// Value representation
// =========================

class ELux;
class Module;
struct Env;
struct ExprBase;
struct ExprVisitor;
struct Iterable;

struct Object;
struct Number;
struct Array;
struct List;
struct Set;
struct Dict;
struct String;
struct Function;
struct Symbol;
struct Pair;

using i64 = std::int64_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using f64 = double;
// using List  = std::list<Value>;
// using Array = std::vector<Value>;
using Self = std::shared_ptr<Object>;
using HSet = std::set<std::string>; // store as stringified values for simplicity
using HMap = std::map<std::string, Self>;
using Context = std::shared_ptr<Env>;
template<typename T>
using Vec = std::vector<T>;
using Expr = std::shared_ptr<ExprBase>;
using NativeFunc = std::function<Self(const Vec<Self>&, Context)>;
using Iterator = std::shared_ptr<Iterable>;

// -*-
struct Object{
    virtual ~Object() = default;
    virtual std::string type(void) const = 0;
    virtual std::string str(void) const = 0;
};

//! @todo
// ----------------
// -*- Iterable -*-
// ----------------
struct Iterable{
    explicit Iterable(Object* data);
    virtual ~Iterable() = default;
    
    virtual Self next(void) = 0;
    virtual bool done(void) const = 0;
    virtual Iterator map(Function func, Context env);
    virtual Iterator filter(Function func, Context env);
    virtual Self reduce(Function func, Context env, const Self& init);
    virtual Iterator zip(Vec<Iterator> iterators, Context env);
    virtual Iterator chain(Vec<Iterator> iterators);
    virtual Iterator take(u32 n);
    virtual Iterator enumerate(Vec<Iterator> iterators);
    virtual Iterator drop_while(Function func, Context env, Vec<Iterator> iterators);
    virtual Iterator take_while(Function func, Context env, Vec<Iterator> iterators);
    virtual bool any(Function func, Context env);
    virtual bool all(Function func, Context env);

private:
    Object* m_data;
};


// -*-
struct Nil final : public Object {
    std::string type(void) const override{
        return "Nil";
    }

    std::string str(void) const override{
        return "nil";
    }
};

// -*-
struct Symbol final : public Object{
    explicit Symbol(const std::string& val);
    Symbol(const Symbol& sym) noexcept;
    Symbol(Symbol&& sym) noexcept;
    Symbol& operator=(const Symbol& sym) noexcept;
    Symbol& operator=(Symbol&& sym) noexcept;

    std::string type(void) const override;
    std::string str(void) const override;
    std::string value;
};

// -*-
struct Pair final : public Object{
    explicit Pair();
    explicit Pair(Self key);
    explicit Pair(Self key, Self val);
    Pair(const Pair& pair) noexcept;
    Pair(Pair&& pair) noexcept;
    Pair& operator=(const Pair& pair) noexcept;
    Pair& operator=(Pair&& pair) noexcept;
    std::string type(void) const override;
    std::string str(void) const override;

    Self key;
    Self val;
};

struct Tuple final: public Object, public Iterable{
    explicit Tuple();
    explicit Tuple(const std::initializer_list<Self>& xs);
    explicit Tuple(const Vec<Self>& xs);
    explicit Tuple(const std::list<Self>& xs);
    explicit Tuple(const Pair& xs);
    explicit Tuple(const List& xs);
    explicit Tuple(const Array& xs);
    explicit Tuple(const Set& xs);
    explicit Tuple(const Dict& xs);
    Tuple(const Tuple& tuple) noexcept;
    Tuple(Tuple&& tuple) noexcept;
    Tuple& operator=(const Tuple& tuple) noexcept;
    Tuple& operator=(Tuple&& tuple) noexcept;
    std::string type(void) const override;
    std::string str(void) const override;
    Vec<Self> items;

    //! @todo
    Self next(void) override;
    bool done(void) const override;
};

// -*-
class ELuxError final: public Object, public std::runtime_error {
public:
    explicit ELuxError();
    explicit ELuxError(const Symbol& sym);
    explicit ELuxError(const Symbol& sym, const std::string& msg);
    ELuxError(const ELuxError& err) noexcept = default;
    ELuxError(ELuxError&& err) noexcept = default;
    ELuxError& operator=(const ELuxError& err) noexcept = default;
    ELuxError& operator=(ELuxError&& err) noexcept = default;

    std::string describe(void) const;
    const Symbol& kind(void) const;
    Symbol& kind(void);
    std::string type(void) const override;
    std::string str(void) const override;

    static Symbol ValueError;
    static Symbol TypeError;
    static Symbol SyntaxError;
    static Symbol RuntimeError;
    static Symbol KeyError;
    static Symbol IndexError;
    
private:
    Symbol m_kind;
};

// -*-
struct Bool final: public Object{
    explicit Bool() : m_val{false}{}
    explicit Bool(bool b) : m_val{b}{}
    Bool(const Bool&) = default;
    Bool(Bool&&) = default;
    Bool& operator=(const Bool&) = default;
    Bool& operator=(Bool&&) = default;
    operator bool() const{ return this->m_val; }

    std::string type(void) const override{
        return "Bool";
    }

    std::string str(void) const override{
        return (this->m_val ? "true" : "false");
    }

private:
    bool m_val;
};

// -*-
struct Number final: public Object {
    explicit Number(): m_val{i64(0)}{}
    explicit Number(i64 num): m_val{num}{}
    explicit Number(f64 num): m_val{num}{}
    Number(const Number&) = default;
    Number(Number&&) = default;
    Number& operator=(const Number&) = default;
    Number& operator=(Number&&) = default;

    operator i64() const{
        i64 num = (
            this->is_integer() ?
            std::get<i64>(m_val) :
            static_cast<i64>(std::get<f64>(m_val))
        );
        return num;
    }

    operator f64() const{
        f64 num = (
            this->is_integer() ?
            static_cast<f64>(std::get<i64>(m_val)) :
            std::get<f64>(m_val)
        );
        return num;
    }

    operator bool() const{
        return (
            this->is_integer() ?
            static_cast<i64>(*this)==0 :
            static_cast<f64>(*this)==0.0
        );
    }

    std::string type(void) const override{
        return (this->is_integer() ? "Integer" : "Float");
    }

    std::string str(void) const override{
        std::stringstream ss;
        ss << (this->is_integer() ? std::get<i64>(m_val) : std::get<f64>(m_val));
        return ss.str();
    }

    bool is_integer(void) const{
        return std::holds_alternative<i64>(this->m_val);
    }

    // -*-
    bool as_bool(void) const;
    i64 as_integer(void) const;
    f64 as_float(void) const;
    Number abs(void) const;
    Number ceil(void) const;
    Number floor(void) const;
    Number round(void) const;
    Number truncate(void) const;
    Number sin(void) const;
    Number cos(void) const;
    Number tan(void) const;
    Number asin(void) const;
    Number acos(void) const;
    Number atan(void) const;
    Number atan2(const Number& rhs) const;
    Number sinh(void) const;
    Number cosh(void) const;
    Number tanh(void) const;
    Number asinh(void) const;
    Number acosh(void) const;
    Number atanh(void) const;
    Number exp(void) const;
    Number expm1(void) const;
    Number exp2(void) const;
    Number pow(const Number& rhs) const;
    Number sqrt(void) const;
    Number cbrt(void) const;
    Number log(void) const;
    Number log2(void) const;
    Number log10(void) const;
    Number log1p(void) const;
    Number erf(void) const;
    Number erfc(void) const;
    Number tgamma(void) const;
    Number lgamma(void) const;
    bool isnan(void) const;
    bool isinf(void) const;
    bool isfinite(void) const;
    Number& operator!();
    Number& operator-();
    Number& operator~();

    friend Number operator+(const Number& lhs, const Number& rhs);
    friend Number operator-(const Number& lhs, const Number& rhs);
    friend Number operator*(const Number& lhs, const Number& rhs);
    friend Number operator/(const Number& lhs, const Number& rhs);
    friend Number operator%(const Number& lhs, const Number& rhs);

    friend Number operator&(const Number& lhs, const Number& rhs);
    friend Number operator|(const Number& lhs, const Number& rhs);
    friend Number operator^(const Number& lhs, const Number& rhs);
    friend Number operator<<(const Number& lhs, const Number& rhs);
    friend Number operator>>(const Number& lhs, const Number& rhs);

    friend bool operator&&(const Number& lhs, const Number& rhs);
    friend bool operator||(const Number& lhs, const Number& rhs);

    friend bool operator==(const Number& lhs, const Number& rhs);
    friend bool operator!=(const Number& lhs, const Number& rhs);
    friend bool operator<(const Number& lhs, const Number& rhs);
    friend bool operator>(const Number& lhs, const Number& rhs);
    friend bool operator<=(const Number& lhs, const Number& rhs);
    friend bool operator>=(const Number& lhs, const Number& rhs);

private:
    using Data = std::variant<i64, f64>;
    Data m_val;
};

// -*-
struct String final: public Object, public Iterable{
    explicit String();// : m_val{std::string()}{}
    explicit String(const std::string& str);//: m_val{std::string(str)}{}
    explicit String(const char* cstr);//: m_val{std::string(cstr)}{}
    explicit String(char c); //: m_val{std::string(1, c)}{}
    String(const String& xs); // : m_val{xs.m_val}{};
    String(String&& xs);//: m_val{std::move(xs.m_val)}{ xs.m_val = {}; }

    //! @todo
    Self next(void) override;
    bool done(void) const override;

    String& operator=(const String& xs){
        if(this != &xs){
            this->m_val = xs.m_val;
        }
        return *this;
    };

    String& operator=(String&& xs){
        if(this != &xs){ this->m_val = std::move(xs.m_val); }
        return *this;
    }
    
    std::string type(void) const override{
        return "String";
    }

    std::string str(void) const override;

    std::string& value(void);
    const std::string& value(void) const;

private:
    std::string m_val;
};

// -*-
struct Set final: public Object, public Iterable {
    explicit Set();
    explicit Set(std::initializer_list<Self> xs);
    explicit Set(const Array& xs);
    explicit Set(const List& xs);
    explicit Set(const Vec<Self>& xs);
    Set(const Set& xs) noexcept;
    Set(Set&& xs) noexcept;
    Set& operator=(const Set& xs) noexcept;
    Set& operator=(Set&& xs) noexcept;

    operator HSet() const;

    std::string type(void) const override;
    std::string str(void) const override;

    HSet& value(void);
    const HSet& value(void) const;

    //! @todo
    Self next(void) override;
    bool done(void) const override;
    
private:
    HSet m_hset;
};

// -*-
struct Dict final: public Object, public Iterable{
    explicit Dict();
    explicit Dict(std::initializer_list<Self> xs);
    explicit Dict(const Array& xs);
    explicit Dict(const List& xs);
    explicit Dict(const Vec<Pair>& pairs);
    Dict(const Dict& xs) noexcept;
    Dict(Dict&& xs) noexcept;
    Dict& operator=(const Dict& xs) noexcept;
    Dict& operator=(Dict&& xs) noexcept;

    operator HMap() const;

    std::string type(void) const override;
    std::string str(void) const override;

    HMap& value(void);
    const HMap& value(void) const;

    //! @todo
    Self next(void) override;
    bool done(void) const override;

private:
    HMap m_hmap;
    friend class ELux;
};

// -*-
struct List final: public Object, public Iterable{
    explicit List();
    explicit List(std::initializer_list<Self> xs);
    explicit List(const Array& xs);
    explicit List(const Set& xs);
    explicit List(const Dict& xs);
    explicit List(const Vec<Self>& xs);
    List(const List& xs) noexcept;
    List(List&& xs) noexcept;
    List& operator=(const List& xs) noexcept;
    List& operator=(List&& xs) noexcept;

    std::string type(void) const override;
    std::string str(void) const override;

    std::list<Self>& value(void);
    const std::list<Self>& value(void) const;

    //! @todo
    Self next(void) override;
    bool done(void) const override;

private:
    std::list<Self> m_xs;
};

// -*-
struct Array final: public Object, public Iterable{
    explicit Array();
    explicit Array(std::initializer_list<Self> xs);
    explicit Array(const List& xs);
    explicit Array(const Set& xs);
    explicit Array(const Dict& xs);
    explicit Array(const Vec<Self>& xs);
    Array(const Array& xs) noexcept;
    Array(Array&& xs) noexcept;
    Array& operator=(const Array& xs) noexcept;
    Array& operator=(Array&& xs) noexcept;

    std::string type(void) const override;
    std::string str(void) const override;

    Vec<Self>& value(void);
    const Vec<Self>& value(void) const;

    Array& push(const Self& self){
        this->m_xs.push_back(self);
        return *this;
    }

    //! @todo
    Self next(void) override;
    bool done(void) const override;

private:
    Vec<Self> m_xs;
};

// -*-
struct Function final: public Object {
    Vec<std::string> params;
    Expr body;
    Context closure;
    bool isMacro = false;
    bool isNative = false;
    NativeFunc native;

    std::string type(void) const override{
        if(this->isMacro){ return "Macro"; }
        if(this->isNative){ return "NativeFn"; }
        return "Function";
    }

    std::string str(void) const override{
        std::stringstream ss;
        ss << (isMacro ? "<Macro @ " : "<function @ ") << std::addressof(*this) << ">";
        return ss.str();
    }

    //! @todo
    Self call(const Vec<Self>& args, Context env);
};

// =========================
// Environment
// =========================

struct Env : std::enable_shared_from_this<Env> {
    std::map<std::string, Self> vars;
    std::set<std::string> immutables;
    std::shared_ptr<Env> parent;

    Env(std::shared_ptr<Env> p = nullptr);
    bool hasLocal(const std::string& name) const;
    void define(const std::string& name, const Self& v);
    bool set(const std::string& name, const Self& v);
    bool contains(const std::string& name) const;
    bool is_immutable(const std::string& name) const;
    Self get(const std::string& name);
};

// =========================
// AST and Visitor
// =========================

struct ExprBase {
    virtual ~ExprBase() = default;
    virtual Self eval(ExprVisitor& v, Context env) = 0;
};

struct LiteralExpr : ExprBase {
    Self value;
    explicit LiteralExpr(const Self& v) : value(v) {}
    Self eval(ExprVisitor& v, Context env) override;
};

struct SymbolExpr : ExprBase {
    std::string name;
    explicit SymbolExpr(std::string n) : name(std::move(n)) {}
    Self eval(ExprVisitor& v, Context env) override;
};

struct ListExpr : ExprBase {
    Vec<Expr> elements;
    Self eval(ExprVisitor& v, Context env) override;
};

struct ExprVisitor {
    virtual ~ExprVisitor() = default;
    virtual Self visit(LiteralExpr& e, Context env) = 0;
    virtual Self visit(SymbolExpr& e, Context env) = 0;
    virtual Self visit(ListExpr& e, Context env) = 0;
};

// =========================
// Tokenizer & Parser
// =========================

enum class TokenKind {
    LPAREN, RPAREN, QUOTE, BACKQUOTE, COMMA, COMMA_AT,
    SYMBOL, STRING, INT, FLOAT, END
};

struct Token {
    TokenKind kind;
    std::string text;
};

struct Lexer {
    explicit Lexer(std::string s);
    Token next();

private:
    std::string m_src;
    size_t m_pos = 0;

    void skipSpaces(void);
    bool startsWith(const std::string& s);
    bool eof(void) const;
    char peek(void) const;
    char peek_next(void) const;
    void advance(void);
    //void skip_comment(void);
};

struct Parser {
    explicit Parser(const std::string& src);
    Vec<Expr> parse(void);

private:
    Lexer m_lexer;
    Token m_token;

    void advance();
    bool match(TokenKind kind);
    Expr parse_expr();
    Expr parse_list();
};

// =====================
// -*- Module System -*-
// =====================
// class Module final{
// public:
// //! @todo

// private:
//     Symbol m_name;

// };

// ==============================
// ELux (Visitor) : the evaluator
// ==============================
class ELux : public ExprVisitor {
public:
    static void run(const std::string& code, Context env, const std::string& label);
    
    static Self share(void);
    static Self share(i64 val);
    static Self share(f64 val);
    static Self share(const Number& val);
    static Self share(const std::string& val);
    static Self share(const char* val);
    static Self share(char val);
    static Self share(const String& val);
    static Self share(bool val);
    static Self share(const Bool& val);
    static Self share(const List& val);
    static Self share(const Array& val);
    static Self share(const Set& val);
    static Self share(const Dict& val);
    static Self share(const Symbol& val);
    static Self share(const Pair& val);
    static Self share(const Tuple& val);
    static Self share(const ELuxError& val);
    static Expr share(const LiteralExpr& expr);
    static Expr share(const SymbolExpr& expr);
    static Expr share(const ListExpr& expr);

    static bool is_nil(const Self& self);
    static bool is_bool(const Self& self);
    static bool is_integer(const Self& self);
    static bool is_float(const Self& self);
    static bool is_number(const Self& self);
    static bool is_string(const Self& self);
    static bool is_list(const Self& self);
    static bool is_array(const Self& self);
    static bool is_set(const Self& self);
    static bool is_dict(const Self& self);
    static bool is_function(const Self& self);
    static bool is_macro(const Self& self);
    static bool is_native(const Self& self);
    static bool is_callable(const Self& self);
    static std::string str(const Self& self);
    static Bool as_bool(const Self& self);
    static f64 as_float(const Self& self);
    static i64 as_integer(const Self& self);
    static Number as_number(const Self& self);
    static String as_string(const Self& self);
    static Array as_array(const Self& self);
    static List as_list(const Self& self);
    static Set as_set(const Self& self);
    static Dict as_dict(const Self& self);
    static Function as_function(const Self& self);
    static bool is_collection(const Self& self);
    static i64 len(const Self& self);

    static std::string repr(const Self& self);

    //! @todo
    /*
    static void check_type(bool pred, const std::string& message);
    static void check_value(bool pred, const std::string& message);
    static void check_syntax(bool pred, const std::string& message);
    static void check_runtime(bool pred, const std::string& message);
    static void check_key(bool pred, const std::string& message);
    static void check_index(bool pred, const std::string& message);
    static void check(bool pred, const std::string& message);
    static void check_argc(bool pred, const std::string& message);

    */

    
public:
    Self visit(LiteralExpr& e, Context env) override;
    Self visit(SymbolExpr& e, Context env) override;
    Self visit(ListExpr& e, Context env) override;
    Self eval(Expr expr, Context env);
    Self eval(const Vec<Expr>& elems, Context env);
    // quasiquote helpers
    Self quasiquote(const Self& v, Context env, int depth = 1);
    Self evalValueAsExpr(const Self& v, Context env);
    Expr valueToExpr(const Self& v);
    Expr valueListToExpr(const List& lst);
    Self expand(const Vec<Expr>& elems, Context env);

private:
    //Self handle_expand(const Vec<Expr>& elems, Context env);
    Self handle_quote(const Vec<Expr>& elems, Context env);
    Self handle_quasiquote(const Vec<Expr>& elems, Context env);
    //Self handle_unquote(const Vec<Expr>& elems, Context env);
    Self handle_if(const Vec<Expr>& elems, Context env);
    Self handle_define(const Vec<Expr>& elems, Context env);
    Self handle_var(const Vec<Expr>& elems, Context env);
    Self handle_lambda(const Vec<Expr>& elems, Context env);
    Self handle_fun(const Vec<Expr>& elems, Context env);
    Self handle_macro(const Vec<Expr>& elems, Context env);
    Self handle_let(const Vec<Expr>& elems, Context env);
    Self handle_progn(const Vec<Expr>& elems, Context env);
    Self handle_while(const Vec<Expr>& elems, Context env);
    Self handle_for(const Vec<Expr>& elems, Context env);
    Self handle_cond(const Vec<Expr>& elems, Context env);
    Self handle_match(const Vec<Expr>& elems, Context env);
    Self handle_try(const Vec<Expr>& elems, Context env);
    Self handle_import(const Vec<Expr>& elems, Context env);
};


// =========================
// Global environment setup
// =========================

void addNative(
    std::shared_ptr<Env> env, const std::string& name,
    std::function<Self(const Vec<Self>&, Context)> fn
);
std::shared_ptr<Env> makeGlobalEnv();


// -*----------------------------------------------------------------*-
}//-*- end::namespace::ekasoft::klx                                 -*-
// -*----------------------------------------------------------------*-

#endif