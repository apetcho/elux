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
#include<optional>
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
struct Hashable;
struct Equalable;
struct Comparable;
struct TotalOrdering;
struct HashHandler;
struct EqualHandler;

struct Object;
struct Number;
struct Array;
struct List;
struct HashSet;
struct HashMap;
struct String;
struct Function;
struct Symbol;
struct Pair;

using i8 = std::int8_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using u32 = std::uint32_t;
using f64 = double;
using usize = std::size_t;
// using List  = std::list<Value>;
// using Array = std::vector<Value>;
using Self = std::shared_ptr<Object>;
// using HSet = std::set<std::string>; // store as stringified values for simplicity
// using HMap = std::map<std::string, Self>;
using Context = std::shared_ptr<Env>;
template<typename T>
using Vec = std::vector<T>;
using Expr = std::shared_ptr<ExprBase>;
using NativeFunc = std::function<Self(const Vec<Self>&, Context)>;
using Iterator = std::shared_ptr<Iterable>;

// -*-
struct HashHandler final{
    usize operator()(const Self& self) const;
};

// -*-
struct EqualHandler{
    bool operator()(const Self& lhs, const Self& rhs) const;
};

using HMap = std::unordered_map<Self, Self, HashHandler, EqualHandler>;
using HSet = std::unordered_set<Self, HashHandler, EqualHandler>;

// -*-
struct Object{
    virtual ~Object() = default;
    virtual Symbol type(void) const = 0;
    virtual std::string str(void) const = 0;
    virtual i64 len(void) const{
        std::stringstream ss;
        ss << "`.len' is not implemented for " << std::quoted(this->type().str());
        ss << " type.";
        throw std::runtime_error(ss.str());
    }
    virtual std::string repr(void) const{
        std::stringstream ss;
        ss << "`.repr' is not implemented for " << std::quoted(this->type().str());
        ss << " type.";
        throw std::runtime_error(ss.str());
    }
};

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
    virtual Iterator zip(Vec<Iterator> iterators);
    virtual Iterator chain(Vec<Iterator> iterators);
    virtual Iterator take(u32 n);
    virtual Iterator drop(u32 n);
    virtual Iterator enumerate(void);
    virtual Iterator drop_while(Function func, Context env);
    virtual Iterator take_while(Function func, Context env);
    virtual bool any(Function func, Context env);
    virtual bool all(Function func, Context env);
    virtual Iterator reverse(void);
    virtual void collect(String& result);
    virtual void collect(Array& result);
    virtual void collect(List& result);
    virtual void collect(Tuple& result);
    virtual void collect(HashSet& result);
    virtual void collect(HashMap& result);

private:
    Object* m_data;
};

// -*-
struct Hashable{
    Hashable(Object* obj): m_data{obj}{}
    virtual ~Hashable() = default;
    virtual usize hash(void) const = 0;

private:
    Object* m_data;
};

// -*-
struct Equalable{
    Equalable(Object* obj);
    virtual ~Equalable() = default;
    virtual bool equal(Object* other) const = 0;
    virtual bool not_equal(Object* other){
        return !this->equal(other);
    }

private:
    Object* m_data;
};

// -*-
struct Comparable{
    Comparable(Object* obj): m_data{obj}{}
    virtual ~Comparable() = default;
    virtual int compare(Object* other) const = 0;

private:
    Object* m_data;
};

// -*-;
struct TotalOrdering : public Equalable, public Comparable{
    TotalOrdering(Object* obj)
    : Equalable(obj), Comparable(obj){}
    virtual ~TotalOrdering() = default;
};


// -*-
struct Nil final : public Object, public Hashable, public Equalable{
    Nil(): Hashable(this), Equalable(this){}
    Nil(const Nil&) noexcept = default;
    Nil(Nil&&) noexcept = default;
    Nil& operator=(const Nil&) noexcept = default;
    Nil& operator=(Nil&&) noexcept = default;

    Symbol type(void) const override;

    std::string str(void) const override{
        return "nil";
    }

    std::string repr(void) const override{
        return this->str();
    }

    usize hash(void) const override;
    bool equal(Object* other) const override;

};

// --------------
// -*- Symbol -*-
// --------------
struct Symbol final : public Object, public Hashable, public Equalable{
    // -*-
    std::string value;

    // -*-
    explicit Symbol(const std::string& val);
    Symbol(const Symbol& sym) noexcept;
    Symbol(Symbol&& sym) noexcept;
    Symbol& operator=(const Symbol& sym) noexcept;
    Symbol& operator=(Symbol&& sym) noexcept;

    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override{
        return this->str();
    }

    usize hash(void) const override;
    bool equal(Object* other) const override;

    friend bool operator==(const Symbol& lhs, const Symbol& rhs);
    friend bool operator!=(const Symbol& lhs, const Symbol& rhs);
};

// ------------
// -*- Pair -*-
// ------------
struct Pair final : public Object, public Hashable, public Equalable{
    explicit Pair(Self key, Self val);
    Pair(const Pair& pair) noexcept;
    Pair(Pair&& pair) noexcept;
    Pair& operator=(const Pair& pair) noexcept;
    Pair& operator=(Pair&& pair) noexcept;
    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;
    usize hash(void) const override;
    bool equal(Object* other) const override;

    Self key;
    Self val;
};

// -------------
// -*- Tuple -*-
// -------------
struct Tuple final: public Object, public Iterable, public Hashable, public Equalable{
    explicit Tuple();
    explicit Tuple(const std::initializer_list<Self>& xs);
    explicit Tuple(const Vec<Self>& xs);
    explicit Tuple(const std::list<Self>& xs);
    explicit Tuple(const Pair& xs);
    explicit Tuple(const List& xs);
    explicit Tuple(const Array& xs);
    explicit Tuple(const HashSet& xs);
    explicit Tuple(const HashMap& xs);
    Tuple(const Tuple& tuple) noexcept;
    Tuple(Tuple&& tuple) noexcept;
    Tuple& operator=(const Tuple& tuple) noexcept;
    Tuple& operator=(Tuple&& tuple) noexcept;
    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;

    i64 len(void) const override{
        return static_cast<i64>(this->m_items.size());
    }

    Self next(void) override;
    bool done(void) const override;

    usize hash(void) const override;
    bool equal(Object* other) const override;

    const Vec<Self>& value(void) const{ return this->m_items; }
    Vec<Self>& value(void){ return this->m_items; }

private:
    Vec<Self> m_items;
    Vec<Self>::iterator m_ptr;
    Vec<Self>::iterator m_stop;
};

// -----------------
// -*- ELuxError -*-
// -----------------
class ELuxError final: public Object, public Hashable, public Equalable{
public:
    explicit ELuxError();
    explicit ELuxError(const std::string& msg);
    explicit ELuxError(const Symbol& sym);
    explicit ELuxError(const Symbol& sym, const std::string& msg);
    ELuxError(const ELuxError& err) noexcept;// = default;
    ELuxError(ELuxError&& err) noexcept;// = default;
    ELuxError& operator=(const ELuxError& err) noexcept;// = default;
    ELuxError& operator=(ELuxError&& err) noexcept;// = default;

    std::string describe(void) const;
    const Symbol& kind(void) const;
    Symbol& kind(void);
    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;

    usize hash(void) const override;
    bool equal(Object* other) const override;

    static Symbol ValueError;
    static Symbol TypeError;
    static Symbol SyntaxError;
    static Symbol RuntimeError;
    static Symbol KeyError;
    static Symbol IndexError;
    
private:
    Symbol m_kind;
    std::string m_msg;
};

// ------------
// -*- Bool -*-
// ------------
struct Bool final: public Object, public Hashable, public TotalOrdering {
    explicit Bool()
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{false}{}
    explicit Bool(bool b)
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{b}{}
    Bool(const Bool& other) noexcept
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{other.m_val}
    {}
    Bool(Bool&& other) noexcept
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{std::move(other.m_val)}
    {}
    Bool& operator=(const Bool&) noexcept;
    Bool& operator=(Bool&&) noexcept;
    operator bool() const{ return this->m_val; }

    Symbol type(void) const override;

    std::string str(void) const override{
        return (this->m_val ? "true" : "false");
    }

    std::string repr(void) const override{
        return this->str();
    }

    usize hash(void) const override;
    bool equal(Object*) const override;
    int compare(Object*) const override;

private:
    bool m_val;
};

// --------------
// -*- Number -*-
// --------------
struct Number final: public Object, public Hashable, public TotalOrdering {
    explicit Number()
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{i64(0)}{}
    explicit Number(i64 num)
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{num}{}
    explicit Number(f64 num)
    : Hashable(this)
    , TotalOrdering(this)
    , m_val{num}{}
    Number(const Number&) noexcept; //= default;
    Number(Number&&) noexcept; //= default;
    Number& operator=(const Number&) noexcept;//= default;
    Number& operator=(Number&&) noexcept;//= default;

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

    Symbol type(void) const override;

    std::string str(void) const override{
        std::stringstream ss;
        ss << (this->is_integer() ? std::get<i64>(m_val) : std::get<f64>(m_val));
        return ss.str();
    }

    std::string repr(void) const override{
        this->str();
    }

    bool is_integer(void) const{
        return std::holds_alternative<i64>(this->m_val);
    }

    usize hash(void) const override;
    bool equal(Object*) const override;
    int compare(Object*) const override;

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

// --------------
// -*- String -*-
// --------------
struct String final: public Object, public Iterable, public Hashable, public TotalOrdering {
    explicit String();
    explicit String(const std::string& str);
    explicit String(const char* cstr);
    explicit String(char c);
    String(const String& xs);
    String(String&& xs);

    Self next(void) override;
    bool done(void) const override;
    usize hash(void) const override;
    bool equal(Object*) const override;
    int compare(Object*) const override;

    String& operator=(const String& xs){
        if(this != &xs){
            this->m_val = xs.m_val;
            this->m_ptr = this->m_val.begin();
            this->m_stop = this->m_val.end();
        }
        return *this;
    };

    String& operator=(String&& xs){
        if(this != &xs){
            this->m_val = std::move(xs.m_val);
            this->m_ptr = this->m_val.begin();
            this->m_stop = this->m_val.end();
            xs.m_val = {};
            xs.m_ptr = xs.m_val.begin();
            xs.m_stop = xs.m_val.end();
        }
        return *this;
    }
    
    Symbol type(void) const override;

    std::string str(void) const override;
    std::string repr(void) const override;

    i64 len(void) const override{
        return static_cast<i64>(this->m_val.length());
    }

    std::string& value(void);
    const std::string& value(void) const;

private:
    std::string m_val;
    std::string::iterator m_ptr;
    std::string::iterator m_stop;
};

// -----------
// -*- Set -*-
// -----------
struct HashSet final: public Object, public Iterable {
    explicit HashSet();
    explicit HashSet(std::initializer_list<Self> xs);
    explicit HashSet(const Array& xs);
    explicit HashSet(const List& xs);
    explicit HashSet(const Vec<Self>& xs);
    HashSet(const HashSet& xs) noexcept;
    HashSet(HashSet&& xs) noexcept;
    HashSet& operator=(const HashSet& xs) noexcept;
    HashSet& operator=(HashSet&& xs) noexcept;

    operator HSet() const;

    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;
    i64 len(void) const override{
        return static_cast<i64>(this->m_hset.size());
    }

    HSet& value(void);
    const HSet& value(void) const;

    Self next(void) override;
    bool done(void) const override;
    
private:
    HSet m_hset;
    HSet::iterator m_ptr;
    HSet::iterator m_stop;
};

// ------------
// -*- Dict -*-
// ------------
struct HashMap final: public Object, public Iterable{
    explicit HashMap();
    explicit HashMap(std::initializer_list<Self> xs);
    explicit HashMap(const Array& xs);
    explicit HashMap(const List& xs);
    explicit HashMap(const Vec<Pair>& pairs);
    HashMap(const HashMap& xs) noexcept;
    HashMap(HashMap&& xs) noexcept;
    HashMap& operator=(const HashMap& xs) noexcept;
    HashMap& operator=(HashMap&& xs) noexcept;

    operator HMap() const;

    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;
    i64 len(void) const override{
        return static_cast<i64>(this->m_hmap.size());
    }

    HMap& value(void);
    const HMap& value(void) const;

    Self next(void) override;
    bool done(void) const override;

private:
    HMap m_hmap;
    HMap::iterator m_ptr;
    HMap::iterator m_stop;
    friend class ELux;
};

// ------------
// -*- List -*-
// ------------
struct List final: public Object, public Iterable{
    explicit List();
    explicit List(std::initializer_list<Self> xs);
    explicit List(const Array& xs);
    explicit List(const HashSet& xs);
    explicit List(const HashMap& xs);
    explicit List(const Vec<Self>& xs);
    List(const List& xs) noexcept;
    List(List&& xs) noexcept;
    List& operator=(const List& xs) noexcept;
    List& operator=(List&& xs) noexcept;

    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;
    i64 len(void) const override{
        return static_cast<i64>(this->m_xs.size());
    }

    std::list<Self>& value(void);
    const std::list<Self>& value(void) const;

    Self next(void) override;
    bool done(void) const override;

private:
    std::list<Self> m_xs;
    std::list<Self>::iterator m_ptr;
    std::list<Self>::iterator m_stop;
};

// -------------
// -*- Array -*-
// -------------
struct Array final: public Object, public Iterable{
    explicit Array();
    explicit Array(std::initializer_list<Self> xs);
    explicit Array(const List& xs);
    explicit Array(const HashSet& xs);
    explicit Array(const HashMap& xs);
    explicit Array(const Vec<Self>& xs);
    Array(const Array& xs) noexcept;
    Array(Array&& xs) noexcept;
    Array& operator=(const Array& xs) noexcept;
    Array& operator=(Array&& xs) noexcept;

    Symbol type(void) const override;
    std::string str(void) const override;
    std::string repr(void) const override;
    i64 len(void) const override{
        return static_cast<i64>(this->m_xs.size());
    }

    Vec<Self>& value(void);
    const Vec<Self>& value(void) const;

    Array& push(const Self& self){
        this->m_xs.push_back(self);
        return *this;
    }

    Self next(void) override;
    bool done(void) const override;

private:
    Vec<Self> m_xs;
    Vec<Self>::iterator m_ptr;
    Vec<Self>::iterator m_stop;
};

// -------------------------------------------
// -*- Function : For <macro, lambda, fun> -*-
// -------------------------------------------
struct Function final: public Object {
    std::optional<std::string> name;
    Vec<std::string> params;
    Expr body = nullptr;
    Context closure;
    bool isMacro = false;
    bool isNative = false;
    NativeFunc native = nullptr;
    ELux* elux;
    //! @todo: add `docstr` field to store optional doc-string

    Symbol type(void) const override;

    std::string str(void) const override;
    std::string repr(void) const override;

    bool is_lambda(void) const{
        return (this->name==std::nullopt);
    }
    //! -*-
    Self call(const Vec<Self>& args, Context env);
    Expr expand(const Vec<Self>& args, Context env);
};

// -----------------
// -*- Operators -*-
// -----------------
Self operator==(const Self& lhs, const Self& rhs);
Self operator!=(const Self& lhs, const Self& rhs);
Self operator<=(const Self& lhs, const Self& rhs);
Self operator>=(const Self& lhs, const Self& rhs);
Self operator<(const Self& lhs, const Self& rhs);
Self operator>(const Self& lhs, const Self& rhs);
Self operator+(const Self& lhs, const Self& rhs);
Self operator-(const Self& lhs, const Self& rhs);
Self operator*(const Self& lhs, const Self& rhs);
Self operator/(const Self& lhs, const Self& rhs);
Self operator%(const Self& lhs, const Self& rhs);
Self operator<<(const Self& lhs, const Self& rhs);
Self operator>>(const Self& lhs, const Self& rhs);
Self operator&(const Self& lhs, const Self& rhs);
Self operator|(const Self& lhs, const Self& rhs);
Self operator^(const Self& lhs, const Self& rhs);
Self operator||(const Self& lhs, const Self& rhs);
Self operator&&(const Self& lhs, const Self& rhs);

// =========================
// Environment
// =========================
//! @todo add `exceptions' field for storing exceptions 
//! @todo add `docstrings' field to handle doctstring define in `var', `define', `fun'
// and `macro' special forms
//! @todo add `types' to store type names defined in the current environment
//! @todo add `modules` to store modules imported in the current environment
struct Env : std::enable_shared_from_this<Env> {
    std::map<std::string, Self> vars;
    std::set<std::string> immutables;
    //! @todo: std::map<std::string, std::string> docstrs;
    //! @todo std::set<std::string> exceptions;
    std::shared_ptr<Env> parent;

    Env(std::shared_ptr<Env> p = nullptr);
    bool hasLocal(const std::string& name) const;
    void define(const std::string& name, const Self& v);
    bool set(const std::string& name, const Self& v);
    bool contains(const std::string& name) const;
    bool is_immutable(const std::string& name) const;
    Self get(const std::string& name);
    //! @todo: std::string get_doc(const std::string&) const;
    //! @todo: void add_doc(const std::string&, const std::string& doc);
};

// =========================
// AST and Visitor
// =========================

struct ExprBase {
    virtual ~ExprBase() = default;
    virtual Self eval(ExprVisitor& v, Context env) = 0;
    virtual std::string repr(void) const = 0;
};

struct LiteralExpr : ExprBase {
    Self value;
    explicit LiteralExpr(const Self& v) : value(v) {}
    Self eval(ExprVisitor& v, Context env) override;
    std::string repr(void) const override;
};

struct SymbolExpr : ExprBase {
    std::string name;
    explicit SymbolExpr(std::string n) : name(std::move(n)) {}
    Self eval(ExprVisitor& v, Context env) override;
    std::string repr(void) const override;
};

struct ListExpr : ExprBase {
    Vec<Expr> elements;
    Self eval(ExprVisitor& v, Context env) override;
    std::string repr(void) const override;
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
    SYMBOL, STRING, INT, FLOAT, END, INVALID,
};

struct Token {
    TokenKind kind = TokenKind::END;
    std::string text = "";
    u32 row;
    u32 col;
};

struct Lexer {
    explicit Lexer(std::string s);
    Token next();

private:
    std::string m_src;
    size_t m_pos = 0;
    u32 m_row = 1;
    u32 m_col = 1;

    void skip(void);
    bool startsWith(const std::string& s);
    bool eof(void) const;
    char peek(void) const;
    char peek_next(void) const;
    void advance(void);
    std::string read_token(void);
    bool is_numeric(const std::string& str, TokenKind& kind);
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

//! @todo
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
    //! @todo: refactor & reimplement this method
    static void run(const std::string& code, Context env, const std::string& label);
    
    //! @todo add `myLicense'
    //! @todo add `myVersion'
    //! @todo add `myAuthors'
    //! @todo add `myNoBanner'
    //! @todo add `myFloatPrecision'
    //! @todo add `myFloatMode'
    //! @todo add `myIntegerMode'
    //! @todo add `myFormatWidth'
    //! @todo add `myModules'
    //! @todo add `myPrelude'
    
    //! @todo: implement the following two methods
    static void repl(const Vec<std::string>& args);
    static void setup(void);

    // --- {File, Path, System, Regex, ...}
    
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
    static Self share(const HashSet& val);
    static Self share(const HashMap& val);
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
    static bool is_hashset(const Self& self);
    static bool is_hashmap(const Self& self);
    static bool is_function(const Self& self);
    static bool is_macro(const Self& self);
    static bool is_native(const Self& self);
    static bool is_callable(const Self& self);
    static bool is_pair(const Self& self);
    static bool is_tuple(const Self& self);
    static bool is_iterable(const Self& self);
    static bool is_hashable(const Self& self);
    static bool is_equalable(const Self& self);
    static bool is_comparable(const Self& self);

    static i64 len(const Self& self){
        return self->len();
    }

    static std::string str(const Self& self);
    static Bool as_bool(const Self& self);
    static f64 as_float(const Self& self);
    static i64 as_integer(const Self& self);
    static Number as_number(const Self& self);
    static String as_string(const Self& self);
    static Array as_array(const Self& self);
    static List as_list(const Self& self);
    static HashSet as_hashset(const Self& self);
    static HashMap as_hashmap(const Self& self);
    static Function as_function(const Self& self);

    static Pair as_pair(const Self& self);
    static Tuple as_tuple(const Self& self);
    static Iterator as_iterator(const Self& self);
    static void collect(Iterator iter, String& result);
    static void collect(Iterator iter, Tuple& result);
    static void collect(Iterator iter, Array& result);
    static void collect(Iterator iter, List& result);
    static void collect(Iterator iter, HashSet& result);
    static void collect(Iterator iter, HashMap& result);

    static bool is_collection(const Self& self);
    static i64 len(const Self& self);

    static void check_type(bool pred, const std::string& message);
    static void check_value(bool pred, const std::string& message);
    static void check_syntax(bool pred, const std::string& message);
    static void check_runtime(bool pred, const std::string& message);
    static void check_key(bool pred, const std::string& message);
    static void check_index(bool pred, const std::string& message);
    static void check(bool pred, const std::string& message);
    static void check_argc(bool pred, const std::string& message);
    
public:
    Self visit(LiteralExpr& e, Context env) override;
    Self visit(SymbolExpr& e, Context env) override;
    Self visit(ListExpr& e, Context env) override;
    Self eval(Expr expr, Context env);
    Self eval(const Vec<Expr>& elems, Context env);
    // quasiquote helpers
    Self quasiquote(const Self& v, Context env, int depth = 1);
    // Self evalValueAsExpr(const Self& v, Context env);
    // Expr valueToExpr(const Self& v);
    // Expr valueListToExpr(const List& lst);
    //Self expand(const Vec<Expr>& elems, Context env);

    Vec<Self> eval_args(const Vec<Expr>& elems, Context env);
    Expr to_expr(const Self& self);
    Expr to_expr(const List& xs);
    Self eval_as_expr(const Self& self, Context env);

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

    //!@todo add `handle_throw' method to handle throw-expressions
    Self handle_throw(const Vec<Expr>& elems, Context env);
    //! @todo add `handle_export' method to handle export-expressions
    Self handle_export(const Vec<Expr>& elems, Context env);
    //! @todo : implement the followings.
    static void initialize_prelude(void);
    static void add_builtin(const std::string& name, NativeFunc func);
    static void add_builtin(const std::string& name, Self self);

    //-------------------------------------------------------------
    //! @note: the following should add builtin data structure APIs
    //-------------------------------------------------------------
    /*
    void initialize_math(void);
    void initialize_string(void);
    void initialize_pair(void);
    void initialize_tuple(void);
    void initialize_array(void);
    void initialize_list(void);
    void initialize_hashset(void);
    void initialize_hashmap(void);
    */

    // ------------------------
    // -*- FUTURE EXTENSION -*-
    // ------------------------
    //! @todo add `handle_use' method to handle use-expressions
    //! @todo add `handle_type' method to handle type-expressions
    //! @todo add `handle_method' method to handle method-expressions
    //! @todo add `handle_trait' method to handle trait-expressions
    //! @todo add `handle_implement' method to handle implement-expressions
    //! @todo add `handle_overload' method to handle overload-expressions
};

/*
struct Type final{
    std::string name;
    std::map<std::string, Self> fields;
    std::map<std::string, Function> methods
    explicit Type(const std::string name);
    void add_field(const std::string&, Self);
    void add_method(const std::string&, Function);
    Self get_field(const std::string&) const;
    Function get_method(const std::string&) const;
    Self call(const std::string&, const Vec<Self>& args);
    Instance operator()(const Vec<Pair>&);
};

struct Instance final: public Object {
    std::shared_ptr<Type> self;

    Self get(const std::string&) const;
    void set(const std::string&, Self);
    void set(const std::string&, Function);
};

struct Trait {
    std::string name;
    std::map<std::string, Pair> func
};
(trait name funcname params [doc])
(implement typename (trait-name funcname params) [doc] body)

eluxNil = ELux::create_type("Nil", ctx);
eluxBool = ELux::create_type("Bool", ctx);
eluxInteger = ELux::create_type("Integer", ctx);
eluxFloat = ELux::create_type("Float", ctx);
eluxSymbol = ELux::create_type("Symbol", ctx);
eluxString = ELux::create_type("String", ctx);
eluxPair = ELux::create_type("Pair", ctx);
eluxTuple = ELux::create_type("Tuple", ctx);
eluxArray = ELux::create_type("Array", ctx);
eluxList = ELux::create_type("List", ctx);
eluxHashMap = ELux::create_type("HashMap", ctx);
eluxHashSet = ELux::create_type("HashSet", ctx);
*/

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