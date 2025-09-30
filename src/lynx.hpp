#ifndef LYNX_HPP
#define LYNX_HPP

#include<filesystem>
#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<cstdint>
#include<variant>
#include<memory>
#include<complex>
#include<vector>
#include<string>
#include<list>
#include<map>
#include<set>

/* 
(define name value)
(define name value docstr)
*/

#define LYNX_KEYWORDS()                                     \
    LYNX_DEF(Cond, "cond")                                  \
    LYNX_DEF(Define, "define")                              \
    LYNX_DEF(For, "for")                                    \
    LYNX_DEF(Fun, "fun")                                    \
    LYNX_DEF(If, "if")                                      \
    LYNX_DEF(Import, "import")                              \
    LYNX_DEF(Lambda, "lambda")                              \
    LYNX_DEF(Let, "let")                                    \
    LYNX_DEF(Macro, "macro")                                \
    LYNX_DEF(Match, "match")                                \
    LYNX_DEF(Progn, "progn")                                \
    LYNX_DEF(Quasiquote, "quasiquote") /* ` */              \
    LYNX_DEF(Quote, "quote") /* ' */                        \
    LYNX_DEF(Unquote, "unquote") /* , */                    \
    LYNX_DEF(UnquoteSplicing, "unquote-splicing") /* ,@ */  \
    LYNX_DEF(Var, "var")                                    \
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

// -*-
namespace fs = std::filesystem;

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -
// Forward class
class Object;
class Symbol;
class Result;

// Type aliases
using Self = std::shared_ptr<Object>;
template<typename T>
using Vec = std::vector<T>;

using f32 = float;
using f64 = double;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using Str = std::string;

using Complex = std::complex<f64>;

using CFun = Result (*)(const Vec<Self>&);
using Dict = std::map<Str, Self>;

// -*---------*-
// --- Error ---
// -*---------*-
class Error final{
public:
    enum class Kind{
        SyntaxError,
        TypeError,
        ValueError,
        RuntimeError,
        FatalError,
    };

    explicit Error(const Str& mgs) noexcept;
    explicit Error(Kind kind, const Str& msg) noexcept;
    explicit Error(Kind kind, const Str& msg, const Self& self) noexcept;
    Error(const Error& err) noexcept;
    Error& operator=(const Error& err) noexcept;
    ~Error() = default;

    Str describe(void) const;

private:
    Kind m_kind;
    Str m_msg;
    Self m_reason;
};

// -----------
// -*- Env -*-
// -----------
class Env final{
public:
    explicit Env() noexcept = default;
    Env(const Env& env) noexcept;
    ~Env() = default;
    bool contains(const Str& key) const;
    void put(const Str& key, const Self& val);
    [[maybe_unused]] Self update(const Str& key, const Self& val);
    Self get(const Str& key) const;

    const Env* parent(void) const;
    Env* parent(void);

private:
    Dict m_bindings;
    Env* m_parent;
};

// --------------
// -*- Result -*-
// --------------
class Result final{
public:
    explicit Result(Self&& self) noexcept;
    explicit Result(Error&& err) noexcept;
    Result(Result&& result) noexcept;
    Result& operator=(Result&& result) noexcept;
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    ~Result() = default;
    bool is_ok(void) const;
    Self ok(void) const;
    Error err(void) const;

private:
    using Value = std::variant<Self, Error>;
    enum class Kind {Ok, Err};
    Value m_value;
};

// ------------------------------
// -*- Object: the base class -*-
// ------------------------------
class Object{
public:
    explicit Object() noexcept = default;
    Object(const Object&) noexcept = default;
    Object(Object&&) noexcept = default;
    Object& operator=(const Object&) = default;
    Object& operator=(Object&&) = default;
    virtual ~Object() = default;

    // -*- Predicates -*-
    virtual bool is_nil(void) const{ return true; }
    virtual bool is_bool(void) const{ return false; }
    virtual bool is_number(void) const{ return false; }
    virtual bool is_integer(void) const{ return false; }
    virtual bool is_float(void) const{ return false; }
    virtual bool is_complex(void) const{ return false; }
    virtual bool is_symbol(void) const{ return false; }
    virtual bool is_string(void) const{ return false; }
    virtual bool is_pair(void) const{ return false; }
    virtual bool is_list(void) const{ return false; }
    virtual bool is_array(void) const{ return false; }
    virtual bool is_hashset(void) const{ return false; }
    virtual bool is_hashmap(void) const{ return false; }
    virtual bool is_callable(void) const{ return false; }
    virtual bool is_builtin(void) const{ return false; }
    virtual bool is_closure(void) const{ return false; }
    virtual bool is_lambda(void) const{ return false; }
    virtual bool is_function(void) const{ return false; }
    virtual bool is_macro(void) const{ return false; }

    virtual Symbol type(void) const;
    virtual Str str(void) const{ return "nil"; }
    virtual Str repr(void) const{ return "nil"; }
};


// ------------
// -*- Bool -*-
// ------------
class Bool final: public Object{
public:
    explicit Bool() noexcept;
    explicit Bool(bool val) noexcept;
    Bool(const Bool&) = default;
    Bool(Bool&&) = default;
    Bool& operator=(const Bool&) = default;
    Bool& operator=(Bool&&) = default;
    ~Bool() = default;

    bool is_nil(void) const override{ return false; }
    bool is_bool(void) const override{ return true; }
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    bool as_bool(void) const;
    i64 as_integer(void) const;
    f64 as_float(void) const;

    bool operator!();

    friend bool operator||(const Bool& lhs, const Bool& rhs);
    friend bool operator&&(const Bool& lhs, const Bool& rhs);

private:
    bool m_value;
};

// --------------------------------------------------------------
// --- Number: wrapper around f64, i64, and std::complex<f64> ---
// --------------------------------------------------------------
class Number final: public Object{
public:
    explicit Number() noexcept;
    explicit Number(f64 num) noexcept;
    explicit Number(i64 num) noexcept;
    explicit Number(f64 x, f64 y) noexcept;
    explicit Number(const Complex& z) noexcept;
    Number(const Number& number) noexcept;
    Number(Number&& number) noexcept;
    Number& operator=(const Number& number) noexcept;
    Number& operator=(Number&& number) noexcept;
    ~Number() = default;

    bool is_nil(void) const override{ return false; }
    bool is_number(void) const override{ return true; }
    bool is_integer(void) const override;
    bool is_float(void) const override;
    bool is_complex(void) const override;
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    bool as_bool(void) const;
    i64 as_integer(void) const;
    f64 as_float(void) const;
    Complex as_complex(void) const;

    Number operator-();

    friend Number operator+(const Number& lhs, const Number& rhs);
    friend Number operator-(const Number& lhs, const Number& rhs);
    friend Number operator*(const Number& lhs, const Number& rhs);
    friend Number operator/(const Number& lhs, const Number& rhs);
    friend Number operator%(const Number& lhs, const Number& rhs);

    friend bool operator==(const Number& lhs, const Number& rhs);
    friend bool operator!=(const Number& lhs, const Number& rhs);
    friend bool operator<=(const Number& lhs, const Number& rhs);
    friend bool operator>=(const Number& lhs, const Number& rhs);
    friend bool operator<(const Number& lhs, const Number& rhs);
    friend bool operator>(const Number& lhs, const Number& rhs);

    Number abs(void) const;
    Number min(const Number& other) const;
    Number max(const Number& other) const;
    Number floor(void) const;
    Number ceil(void) const;
    Number round(void) const;
    Number log(void) const;
    Number log2(void) const;
    Number log10(void) const;
    Number log1p(void) const;
    Number exp(void) const;
    Number exp2(void) const;
    Number expm1(void) const;
    Number pow(const Number& other) const;
    Number sqrt(void) const;
    Number cbrt(void) const;
    Number hypot(const Number& other) const;
    Number sin(void) const;
    Number cos(void) const;
    Number tan(void) const;
    Number asin(void) const;
    Number acos(void) const;
    Number atan(void) const;
    Number atan2(void) const;
    Number sinh(void) const;
    Number cosh(void) const;
    Number tanh(void) const;
    Number asinh(void) const;
    Number acosh(void) const;
    Number atanh(void) const;
    Number erf(void) const;
    Number erfc(void) const;
    Number tgamma(void) const;
    Number lgamma(void) const;
    Number isfinite(void) const;
    Number isinf(void) const;
    Number isnan(void) const;

    static Number PI;
    static Number E;
    static Number FMAX;
    static Number FMIN;
    static Number IMAX;
    static Number IMIN;
    static Number EPSILON;
    static Number PHI;
    static Number NaN;

private:
    enum class Kind {INT, FLOAT, COMPLEX };
    using Value = std::variant<i64, f64, Complex>;

    Kind m_kind;
    Value m_value;

    void get(f64& num){ num = std::get<f64>(this->m_value); }
    void get(i64& num){ num = std::get<i64>(this->m_value); }
    void get(Complex& znum){ znum = std::get<Complex>(this->m_value); }
    void get(f64& num) const{ num = std::get<f64>(this->m_value); }
    void get(i64& num) const{ num = std::get<i64>(this->m_value); }
    void get(Complex& znum) const{ znum = std::get<Complex>(this->m_value); }
};

// --------------
// -*- Symbol -*-
// --------------
class Symbol final: public Object{
public:
    explicit Symbol(const Str& str) noexcept;
    Symbol(const Symbol& other) noexcept = default;
    Symbol(Symbol&& other) noexcept = default;
    Symbol& operator=(const Symbol&) noexcept = default;
    Symbol& operator=(Symbol&&) noexcept = default;
    ~Symbol() = default;

    bool is_nil(void) const override{ return false; }
    bool is_symbol(void) const override{ return true; }
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    friend bool operator==(const Symbol& lhs, const Symbol& rhs);
    friend bool operator!=(const Symbol& lhs, const Symbol& rhs);

private:
    Str m_str;
};

// --------------
// -*- String -*-
// --------------
class String final: public Object{
public:
    explicit String() noexcept;
    explicit String(const Str& str) noexcept;
    String(const String& other) noexcept;
    String(String&& other) noexcept;
    ~String() = default;

    bool is_nil(void) const override{ return false; }
    bool is_string(void) const override{ return true; }

    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    friend bool operator+(const String& lhs, const String& rhs);
    friend bool operator==(const String& lhs, const String& rhs);
    friend bool operator!=(const String& lhs, const String& rhs);
    friend bool operator<=(const String& lhs, const String& rhs);
    friend bool operator>=(const String& lhs, const String& rhs);
    friend bool operator<(const String& lhs, const String& rhs);
    friend bool operator>(const String& lhs, const String& rhs);

    i64 len(void) const;
    String capitalize(void) const;
    String upper(void) const;
    String lower(void) const;
    String trim(void) const;
    String ltrim(void) const;
    String rtrim(void) const;
    String join(const Vec<String>& vec) const;
    String replace(const String& old, const String& neo) const;
    Vec<String> split(const String& delim=String(" "));
    bool contains(const String& needle) const;
    i64 find(const String& needle) const;
    String substr(i64 start=0, i64 end=Str::npos) const;

private:
    Str m_str;
};

// ------------
// -*- List -*-
// ------------
class List final: public Object{
public:
    explicit List() noexcept = default;
    explicit List(const Vec<Self>& xs) noexcept;
    explicit List(const std::list<Self>& xs) noexcept;
    List(const List& other) noexcept = default;
    List(List&& other) noexcept = default;
    List& operator=(const List& ) noexcept = default;
    List& operator=(List&& ) noexcept = default;
    ~List() = default;

    bool is_nil(void) const override;
    bool is_list(void) const override;
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    std::list<Self> as_list(void) const;
    Vec<Self> as_vector(void) const;

    i64 len(void) const;
    Self head(void) const;
    List tail(void) const;
    Self last(void) const;
    Self nth(void) const;
    Self push(void);
    Self pop(void);
    Self append(void);
    Self insert(i64 idx, const Self& self);
    Self remove(i64 idx);
    
    friend bool operator+(const List& lhs, const List& rhs);


private:
    using Value = std::list<Self>;
    Value m_value;
};

// ---------------
// -*- Builtin -*-
// ---------------
class Builtin final: public Object{
public:
    explicit Builtin(const Str& name, CFun cfun, i32 minArgc, i32 maxArgc) noexcept;
    Builtin(const Builtin& builtin) noexcept;
    Builtin(Builtin&& builtin) noexcept;
    Builtin& operator=(const Builtin& builtin) noexcept;
    Builtin& operator=(Builtin&& builtin) noexcept;
    ~Builtin() = default;

    bool is_nil(void) const override{ return false; }
    bool is_callable(void) const override{ return true; }
    bool is_builtin(void) const override{ return true; }
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    i64 min_argc(void) const;
    i64 max_argc(void) const;
    const Str& name(void) const;

    Result operator()(const Vec<Self>& args);

private:
    Str m_name;
    CFun m_cfun;
    i64 m_minArgc;
    i64 m_maxArgc;
};


// ---------------
// --- Closure ---
// ---------------
class Closure final: public Object{
public:
    explicit Closure(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept;
    Closure(const Closure& closure) noexcept;
    Closure(Closure&& closure) noexcept;
    Closure& operator=(const Closure& closure) noexcept;
    Closure& operator=(Closure&& closure) noexcept;
    ~Closure() = default;

    bool is_nil(void) const override{ return false; }
    bool is_callable(void) const override;
    bool is_lambda(void) const override;
    bool is_function(void) const override;
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    i64 argc(void) const;
    Result operator()(const Vec<Self>& args);

    const Str& name(void) const{ return this->m_name; }
    const Vec<Symbol>& params(void) const{ return this->m_params; }
    const Vec<Self>& body(void) const{ return this->m_body; }

private:
    enum class Kind {LAMBDA, FUNC};
    Kind m_kind;
    Str m_name;
    Vec<Symbol> m_params;
    Vec<Self> m_body;
    Env m_env;
};

// -------------
// --- Macro ---
// -------------
class Macro final: public Object{
public:
    explicit Macro(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept;
    Macro(const Macro& macro) noexcept;
    Macro(Closure&& macro) noexcept;
    Macro& operator=(const Macro& macro) noexcept;
    Macro& operator=(Macro&& macro) noexcept;
    ~Macro() = default;

    bool is_nil(void) const override{ return false; }
    bool is_callable(void) const override{ return true; }
    bool is_macro(void) const override{ return true; }
    Symbol type(void) const override;
    Str str(void) const override;
    Str repr(void) const override;

    i64 argc(void) const;
    Self expand(const Vec<Self>& args) const;
    Result operator()(const Vec<Self>& args);
    const Str& name(void) const{ return this->m_name; }
    const Vec<Symbol>& params(void) const{ return this->m_params; }
    const Vec<Self>& body(void) const{ return this->m_body; }

private:
    Str m_name;
    Vec<Symbol> m_params;
    Vec<Self> m_body;
    Env m_env;
};

/** @todo
// class Pair final: public Object{};
// class HashSet final: public Object{};
// class HashMap final: public Object{};
// class Array final: public Object{};
*/

// -*-
enum class TokenKind{
#define LYNX_DEF(tok, _) tok,
    LYNX_TOKENS()
#undef LYNX_DEF
};

struct Token final{
    TokenKind kind = TokenKind::Invalid;
    Str lexeme{};
    i64 row{};
    i64 col{};

    explicit Token() noexcept = default;
    explicit Token(TokenKind kd, Str str, i64 r, i64 c)
    : kind{kd}, lexeme{str}, row{r}, col{c}{}
};

// -*-------------*-
// -*- Tokenizer -*-
// -*-------------*-
class Tokenizer final{
public:
    explicit Tokenizer() noexcept = default;
    explicit Tokenizer(std::istringstream&& stream) noexcept;
    explicit Tokenizer(std::ifstream&& stream) noexcept;
    Tokenizer(const Tokenizer&) = delete;
    Tokenizer& operator=(const Tokenizer&) = delete;
    Tokenizer(Tokenizer&& tokenizer) noexcept;
    Tokenizer& operator=(Tokenizer&& tokenizer) noexcept;
    ~Tokenizer() = default;

    Token token(void);

private:
    enum class Kind{STR, FILE};
    Kind m_kind;
    std::istringstream m_sstream;
    std::ifstream m_fstream;

    bool is_symbol_char(i32 c);
    bool is_syntax_quote(i32 c);
    bool is_reserved_word(const Str& word);
    void skip_whitespace(void);
    void peek(i32 idx=0);
    void advance(i32 count=1);

    Token read_symbol(void);
    Token read_f64_or_i64(void);
    Token read_string(void);
};

// --------------
// -*- Parser -*-
// --------------
class Parser final{
public:
    explicit Parser() noexcept = default;
    explicit Parser(std::istringstream&& stream) noexcept;
    explicit Parser(std::ifstream&& stream) noexcept;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete; 
    Parser(Parser&& parser) noexcept;
    Parser& operator=(Parser&& parser) noexcept;
    ~Parser() = default;

    Self parse(void);

private:
    Tokenizer m_tokenizer;

    Self parse_atom(void);
    Self parse_list(void);
};

// -*----------*-
// --- Module ---
// -*----------*-
class Module final{
public:
    explicit Module(const Str& name, const fs::path& path, Env* env) noexcept;
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&& other) noexcept;
    Module& operator=(Module&& other) noexcept;
    ~Module() = default;
    const Symbol& name(void) const;
    const fs::path& path(void) const;
    const Env& env(void) const;

private:
    Symbol m_name;
    fs::path m_path;
    Env m_env;

    void initialize(void);
};

// -----------------------------
// -*- Lynx: the interpreter -*-
// -----------------------------
class Lynx final{
public:
    explicit Lynx() noexcept = default;

    static Env prelude;
    static std::map<Str, Module> libraries;

    const Env& runtime(void) const;
    Env& runtime(void);

    static void repl(void);
    static void run(const Vec<Str>& args);

    void setup(void);

private:
    Env m_runtime;
    std::map<Str, Module> m_imported();

    Str make_key(const Module& mod);
    static void initialize_prelude(void);
    static void initialize_math_lib(void);

    static bool check_argc(int argc, int expected, const Str& funcname, Error& err);
    static bool check_type(const Symbol& ty, const Self& self, Error& err);

    bool is_keyword(const Str& word);
    Self eval(const Self& self, Env& env);

    // -*-
    Result handle_cond(const Self& self, Env& env);
    Result handle_define(const Self& self, Env& env);
    Result handle_for(const Self& self, Env& env);
    Result handle_fun(const Self& self, Env& env);
    Result handle_if(const Self& self, Env& env);
    Result handle_import(const Self& self, Env& env);
    Result handle_lambda(const Self& self, Env& env);
    Result handle_let(const Self& self, Env& env);
    Result handle_macro(const Self& self, Env& env);
    Result handle_progn(const Self& self, Env& env);
    Result handle_quote(const Self& self, Env& env);
    Result handle_quasiquote(const Self& self, Env& env);
    Result handle_unquote(const Self& self, Env& env);
    Result handle_unquote_splicing(const Self& self, Env& env);
    Result handle_match(const Self& self, Env& env);
    Result handle_var(const Self& self, Env& env);
    Result handle_while(const Self& self, Env& env);
    Result eval_atom(const Self& self, Env& env);
    Result eval_list(const Self& self, Env& env);

public:
    // Constructors
    static Result fn_bool(const Vec<Self>& args);
    static Result fn_integer(const Vec<Self>& args);
    static Result fn_float(const Vec<Self>& args);
    static Result fn_complex(const Vec<Self>& args);
    static Result fn_string(const Vec<Self>& args);
    static Result fn_list(const Vec<Self>& args);

    // Predicates
    static Result fn_is_bool(const Vec<Self>& args);
    static Result fn_is_number(const Vec<Self>& args);
    static Result fn_is_integer(const Vec<Self>& args);
    static Result fn_is_float(const Vec<Self>& args);
    static Result fn_is_complex(const Vec<Self>& args);
    static Result fn_is_string(const Vec<Self>& args);
    static Result fn_is_list(const Vec<Self>& args);
    static Result fn_is_callable(const Vec<Self>& args);
    static Result fn_is_builtin(const Vec<Self>& args);
    static Result fn_is_closure(const Vec<Self>& args);
    static Result fn_is_lambda(const Vec<Self>& args);
    static Result fn_is_function(const Vec<Self>& args);

    // I/O functions
    static Result fn_print(const Vec<Self>& args);
    static Result fn_eprint(const Vec<Self>& args);
    static Result fn_println(const Vec<Self>& args);
    static Result fn_eprintln(const Vec<Self>& args);
    static Result fn_input(const Vec<Self>& args);
    static Result fn_format(const Vec<Self>& args);

    // Arithmetic operators
    static Result fn_add(const Vec<Self>& args);
    static Result fn_sub(const Vec<Self>& args);
    static Result fn_mul(const Vec<Self>& args);
    static Result fn_div(const Vec<Self>& args);
    static Result fn_mod(const Vec<Self>& args);

    // Relational operators
    static Result fn_lt(const Vec<Self>& args);
    static Result fn_le(const Vec<Self>& args);
    static Result fn_gt(const Vec<Self>& args);
    static Result fn_ge(const Vec<Self>& args);
    static Result fn_eq(const Vec<Self>& args);
    static Result fn_ne(const Vec<Self>& args);

    // Logical operators
    static Result fn_and(const Vec<Self>& args);
    static Result fn_or(const Vec<Self>& args);
    static Result fn_not(const Vec<Self>& args);

    // Functional APIs
    static Result fn_map(const Vec<Self>& args);
    static Result fn_zip(const Vec<Self>& args);
    static Result fn_filter(const Vec<Self>& args);
    static Result fn_reduce(const Vec<Self>& args);
    static Result fn_take(const Vec<Self>& args);
    static Result fn_take_while(const Vec<Self>& args);

    // Common list & string functions
    static Result fn_len(const Vec<Self>& args);
    static Result fn_concat(const Vec<Self>& args);

    // Functions on list
    static Result fn_head(const Vec<Self>& args);
    static Result fn_tail(const Vec<Self>& args);
    static Result fn_nth(const Vec<Self>& args);
    static Result fn_insert(const Vec<Self>& args);
    static Result fn_remove(const Vec<Self>& args);
    static Result fn_push(const Vec<Self>& args);
    static Result fn_pop(const Vec<Self>& args);
    static Result fn_append(const Vec<Self>& args);

    // Functions on string
    static Result fn_capitalize(const Vec<Self>& args);
    static Result fn_upper(const Vec<Self>& args);
    static Result fn_lower(const Vec<Self>& args);
    static Result fn_contains(const Vec<Self>& args);
    static Result fn_find(const Vec<Self>& args);
    static Result fn_(const Vec<Self>& args);

    // Mathetical functions
    static Result fn_abs(const Vec<Self>& args);
    static Result fn_min(const Vec<Self>& args);
    static Result fn_max(const Vec<Self>& args);
    static Result fn_ceil(const Vec<Self>& args);
    static Result fn_floor(const Vec<Self>& args);
    static Result fn_round(const Vec<Self>& args);
    static Result fn_log(const Vec<Self>& args);
    static Result fn_log2(const Vec<Self>& args);
    static Result fn_log10(const Vec<Self>& args);
    static Result fn_log1p(const Vec<Self>& args);
    static Result fn_exp(const Vec<Self>& args);
    static Result fn_exp2(const Vec<Self>& args);
    static Result fn_expm1(const Vec<Self>& args);
    static Result fn_pow(const Vec<Self>& args);
    static Result fn_sqrt(const Vec<Self>& args);
    static Result fn_cbrt(const Vec<Self>& args);
    static Result fn_sin(const Vec<Self>& args);
    static Result fn_cos(const Vec<Self>& args);
    static Result fn_tan(const Vec<Self>& args);
    static Result fn_asin(const Vec<Self>& args);
    static Result fn_acos(const Vec<Self>& args);
    static Result fn_atan(const Vec<Self>& args);
    static Result fn_atan2(const Vec<Self>& args);
    static Result fn_erf(const Vec<Self>& args);
    static Result fn_erfc(const Vec<Self>& args);
    static Result fn_tgamma(const Vec<Self>& args);
    static Result fn_lgamma(const Vec<Self>& args);
    static Result fn_isfinite(const Vec<Self>& args);
    static Result fn_isinf(const Vec<Self>& args);
    static Result fn_isnan(const Vec<Self>& args);

    // Miscellaneous functions
    static Result fn_random(const Vec<Self>& args);
    static Result fn_range(const Vec<Self>& args);
    static Result fn_linspace(const Vec<Self>& args);
    static Result fn_sort(const Vec<Self>& args);
    static Result fn_now(const Vec<Self>& args);
    static Result fn_today(const Vec<Self>& args);
    static Result fn_sleep(const Vec<Self>& args);
    static Result fn_timeit(const Vec<Self>& args);
    static Result fn_eval(const Vec<Self>& args);
};


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-

#endif