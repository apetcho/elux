#ifndef LYNX_HPP
#define LYNX_HPP

#include<initializer_list>
#include<functional>
#include<algorithm>
#include<stdexcept>
#include<optional>
#include<cstdint>
#include<iostream>
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
struct Env;
struct ExprBase;
struct ExprVisitor;

struct Object;
struct Number;
struct Array;
struct List;
struct Set;
struct Dict;
struct String;
struct Function;

using i64 = std::int64_t;
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

struct Object{
    virtual ~Object() = default;
    virtual std::string type(void) const = 0;
    virtual std::string str(void) const = 0;
};

struct Nil final : public Object {
    std::string type(void) const override{
        return "Nil";
    }

    std::string str(void) const override{
        return "nil";
    }
};

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
    explicit Number(): m_fixed{true}, m_val{i64(0)}{}
    explicit Number(i64 num): m_fixed{true}, m_val{num}{}
    explicit Number(f64 num): m_fixed{false}, m_val{num}{}
    Number(const Number&) = default;
    Number(Number&&) = default;
    Number& operator=(const Number&) = default;
    Number& operator=(Number&&) = default;

    operator i64() const{
        i64 num = (
            this->m_fixed ?
            std::get<i64>(m_val) :
            static_cast<i64>(std::get<f64>(m_val))
        );
        return num;
    }

    operator f64() const{
        f64 num = (
            this->m_fixed ?
            static_cast<f64>(std::get<i64>(m_val)) :
            std::get<f64>(m_val)
        );
        return num;
    }

    operator bool() const{
        return (
            this->m_fixed ?
            static_cast<i64>(*this)==0 :
            static_cast<f64>(*this)==0.0
        );
    }

    std::string type(void) const override{
        return (this->m_fixed ? "Integer" : "Float");
    }

    std::string str(void) const override{
        std::stringstream ss;
        ss << (this->m_fixed? std::get<i64>(m_val) : std::get<f64>(m_val));
        return ss.str();
    }

    bool is_integer(void) const{ return this->m_fixed; }

private:
    using Data = std::variant<i64, f64>;
    bool m_fixed{};
    Data m_val;
};

// -*-
struct String final: public Object{
    explicit String() : m_val{std::string()}{}
    explicit String(const std::string& str): m_val{std::string(str)}{}
    explicit String(const char* cstr): m_val{std::string(cstr)}{}
    explicit String(char c): m_val{std::string(1, c)}{}
    String(const String& xs) : m_val{xs.m_val}{};
    String(String&& xs): m_val{std::move(xs.m_val)}{
        xs.m_val = {};
    }
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

    std::string str(void) const override{
        return this->m_val;
    }

    std::string& value(void);
    const std::string& value(void) const;

private:
    std::string m_val;
};

// -*-
struct Set final: public Object {
    explicit Set();
    explicit Set(std::initializer_list<Self> xs);
    explicit Set(const Array& xs);
    explicit Set(const List& xs);
    Set(const Set& xs) noexcept;
    Set(Set&& xs) noexcept;
    Set& operator=(const Set& xs) noexcept;
    Set& operator=(Set&& xs) noexcept;

    operator HSet() const;

    std::string type(void) const override;
    std::string str(void) const override;

    HSet& value(void);
    const HSet& value(void) const;
    
private:
    HSet m_hset;
};

// -*-
struct Dict final: public Object {
    explicit Dict();
    explicit Dict(std::initializer_list<Self> xs);
    explicit Dict(const Array& xs);
    explicit Dict(const List& xs);
    Dict(const Dict& xs) noexcept;
    Dict(Dict&& xs) noexcept;
    Dict& operator=(const Dict& xs) noexcept;
    Dict& operator=(Dict&& xs) noexcept;

    operator HMap() const;

    std::string type(void) const override;
    std::string str(void) const override;

    HMap& value(void);
    const HMap& value(void) const;

private:
    HMap m_hmap;
    friend class ELux;
};

// -*-
struct List final: public Object {
    explicit List();
    explicit List(std::initializer_list<Self> xs);
    explicit List(const Array& xs);
    explicit List(const Set& xs);
    explicit List(const Dict& xs);
    List(const List& xs) noexcept;
    List(List&& xs) noexcept;
    List& operator=(const List& xs) noexcept;
    List& operator=(List&& xs) noexcept;

    std::string type(void) const override;
    std::string str(void) const override;

    std::list<Self>& value(void);
    const std::list<Self>& value(void) const;

private:
    std::list<Self> m_xs;
};

// -*-
struct Array final: public Object {
    explicit Array();
    explicit Array(std::initializer_list<Self> xs);
    explicit Array(const List& xs);
    explicit Array(const Set& xs);
    explicit Array(const Dict& xs);
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