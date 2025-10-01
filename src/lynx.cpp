#include "lynx.hpp"

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -

// -------------
// -*- Error -*-
// -------------
Error::Error(const Str& msg) noexcept
: m_kind{Kind::FatalError}
, m_msg{msg}
, m_reason{nullptr}{
    this->m_prefix = this->make_prefix();
}

// -*-
Error::Error(Kind kind, const Str& msg) noexcept
: m_kind{kind}
, m_msg{msg}
, m_reason{nullptr}{
    this->m_prefix = this->make_prefix();
}

// -*-
Error::Error(Kind kind, const Str& msg, const Self& self) noexcept
: m_kind{kind}
, m_msg{msg}
, m_reason{self}{
    this->m_prefix = this->make_prefix();
}

// -*-
Error::Error(const Error& err) noexcept
: m_kind{err.m_kind}
, m_msg{err.m_msg}
, m_reason{err.m_reason}
, m_prefix{err.m_prefix}
{}

// -*-
Error::Error(Error&& err) noexcept
: m_kind{std::move(err.m_kind)}
, m_msg{std::move(err.m_msg)}
, m_reason{std::move(err.m_reason)}
, m_prefix{std::move(err.m_prefix)}
{}

// -*-
Error& Error::operator=(const Error& err) noexcept{
    if(this != &err){
        this->m_kind = err.m_kind;
        this->m_msg = err.m_msg;
        this->m_reason = err.m_reason;
        this->m_prefix = err.m_prefix;
    }

    return *this;
}

// -*-
Error& Error::operator=(Error&& err) noexcept{
    if(this != &err){
        this->m_kind = std::move(err.m_kind);
        this->m_msg = std::move(err.m_msg);
        this->m_reason = std::move(err.m_reason);
        this->m_prefix = std::move(err.m_prefix);
    }

    return *this;
}

// -*-
Str Error::describe(void) const{
    std::stringstream ss;
    ss << this->m_prefix << ":\n" << this->m_msg;
    if(this->m_reason != nullptr){
        ss << "\n";
        ss << this->m_reason->str();
    }

    return ss.str();
}

// -*-
const Str& Error::prefix(void) const{
    return this->m_prefix;
}

// -*-
Str& Error::prefix(void){
    return this->m_prefix;
}

// -*-
Str Error::make_prefix(void){
    std::map<Error::Kind, Str> emap = {
        {Error::Kind::FatalError, "Error"},
        {Error::Kind::RuntimeError, "RuntimeError"},
        {Error::Kind::SyntaxError, "SyntaxError"},
        {Error::Kind::TypeError, "TypeError"},
        {Error::Kind::ValueError, "ValueError"},
    };
    auto entry = emap.find(this->m_kind);
    return entry->second;
}

// -----------
// -*- Env -*-
// -----------
Env::Env(const Env& env) noexcept
: m_bindings{env.m_bindings}
, m_parent{env.m_parent}
{}

// -*-
bool Env::contains(const Str& key) const{
    auto entry = this->m_bindings.find(key);
    if(entry != this->m_bindings.end()){
        return true;
    }else{
        if(this->m_parent == nullptr){
            return false;
        }
        return this->m_parent->contains(key);
    }
}

// -*-
void Env::put(const Str& key, const Self& val){
    this->m_bindings[key] = val;
}

// -*-
[[maybe_unused]] Self Env::update(const Str& key, const Self& val){
    if(!this->contains(key)){
        std::stringstream ss;
        ss << "unbound identifier `" << key << "'. Cannot be updated.";
        throw Error(Error::Kind::RuntimeError, ss.str());
    }
    if(this->m_bindings.find(key) != this->m_bindings.end()){
        this->m_bindings[key] = val;
    }

    return this->m_parent->update(key, val);
}

// -*-
Self Env::get(const Str& key) const{
    if(!this->contains(key)){
        return nullptr;
    }
    if(this->m_bindings.find(key) != this->m_bindings.end()){
        auto self = this->m_bindings.at(key);
        return self;
    }
    return this->m_parent->get(key);
}

// -*-
const Env* Env::parent(void) const{
    return this->m_parent;
}

// -*-
Env* Env::parent(void){
    return this->m_parent;
}

// --------------
// -*- Result -*-
// --------------
Result::Result(Self&& self) noexcept
: m_kind{Kind::Ok}
, m_value{std::move(self)}
{}

// -*-
Result::Result(Error&& err) noexcept
: m_kind{Kind::Err}
, m_value{std::move(err)}
{}

Result::Result(Result&& result) noexcept
: m_kind{std::move(result.m_kind)}
, m_value{std::move(result.m_value)}
{}

// -*-
Result& Result::operator=(Result&& result) noexcept{
    if(this != &result){
        this->m_kind = std::move(result.m_kind);
        this->m_value = std::move(result.m_value);
    }
    return *this;
}

// -*-
bool Result::is_ok(void) const{
    return this->m_kind == Kind::Ok;
}

// -*-
Self Result::ok(void) const{
    Self self = nullptr;
    self = std::get<Self>(this->m_value);
    return self;
}

// -*-
Error Result::err(void) const{
    Error error;
    error = std::get<Error>(this->m_value);
    return error;
}

// -*----------*-
// --- Module ---
// -*----------*-
Module::Module(const Str& name, Env* env) noexcept
: m_name{name}
, m_path{fs::path("@lynx")} // for builtin modules
, m_env{Env()}{
    *this->m_env.parent() = *env;
    this->initialize();
}

// -*-
Module::Module(const Str& name, const fs::path& path, Env* env) noexcept
: m_name{name}
, m_path{path}
, m_env{Env()}{
    *this->m_env.parent() = *env;
    this->initialize();
}

Module::Module(Module&& other) noexcept
: m_name{std::move(other.m_name)}
, m_path{std::move(other.m_path)}
, m_env{std::move(other.m_env)}
{}

// -*-
Module& Module::operator=(Module&& other) noexcept{
    if(this != &other){
        this->m_name = std::move(other.m_name);
        this->m_path = std::move(other.m_path);
        this->m_env = std::move(other.m_env);
    }
    return *this;
}

// -*-
Symbol Module::name(void) const{
    return Symbol(this->m_name);
}

/*
const fs::path& Module::path(void) const;
const Env& Module::env(void) const;

void Module::initialize(void);
*/


// -----------------------------
// -*- Lynx: the interpreter -*-
// -----------------------------
/*
Env Lynx::lynxDocs;
Env Lynx::docstrs;
Env Lynx::prelude;
std::map<Str, Module> Lynx::libraries;

const Env& Lynx::runtime(void) const;
Env& Lynx::runtime(void);
void Lynx::repl(void);
void Lynx::run(const Vec<Str>& args);
void Lynx::setup(void);
Result Lynx::eval(const Self& self, Env& env);

Str Lynx::readfile(const Str& filename){
    if(!fs::exists(fs::path(filename))){
        std::stringstream ss;
        ss << "file '" << filename << "' not found";
        throw Error(Error::Kind::RuntimeError, ss.str());
    }
    std::ifstream fin(filename);
    Str result{};
    if(!fin.is_open()){
        std::stringstream ss;
        ss << "error opening the file '" << filename << "'";
        throw Error(Error::Kind::RuntimeError, ss.str());
    }
    fin.seekg(0, std::ios::end);
    result.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);
    result.assign(
        std::istreambuf_iterator<char>(fin),
        std::istreambuf_iterator<char>()
    );

    fin.close();
    return result;
}
Str Lynx::make_library_key(const Module& mymodule);
void Lynx::push_module(const Module& mymodule);
void Lynx::push_module(const Str& name, const Module& mymodule);
void Lynx::import_module(const Str& module_name);
void Lynx::import_module(const fs::path& module_path);

bool Lynx::check_argc(int argc, int expected, const Str& funcname, Error& err){}
bool Lynx::check_type(const Symbol& ty, const Self& self, Error& err){}
bool Lynx::check_value(const Self& self, bool (*fn)(const Self&), Error& err){}
bool Lynx::check_value(const Self& self, bool pred, Error& err){}

bool Lynx::is_reserved_word(const Str& word){}
bool Lynx::is_keyword(const Str& word){}
Vec<Symbol> Lynx::captured_symbols(const Vec<Self>& body){}

Result Lynx::handle_cond(const Self& self, Env& env){}
Result Lynx::handle_defvar(const Self& self, Env& env){}
Result Lynx::handle_for(const Self& self, Env& env){}
Result Lynx::handle_fun(const Self& self, Env& env){}
Result Lynx::handle_if(const Self& self, Env& env){}
Result Lynx::handle_import(const Self& self, Env& env){}
Result Lynx::handle_lambda(const Self& self, Env& env){}
Result Lynx::handle_let(const Self& self, Env& env){}
Result Lynx::handle_macro(const Self& self, Env& env){}
Result Lynx::handle_progn(const Self& self, Env& env){}
Result Lynx::handle_quote(const Self& self, Env& env){}
Result Lynx::handle_quasiquote(const Self& self, Env& env){}
Result Lynx::handle_unquote(const Self& self, Env& env){}
Result Lynx::handle_unquote_splicing(const Self& self, Env& env){}

Result Lynx::handle_match(const Self& self, Env& env){}
Result Lynx::handle_var(const Self& self, Env& env){}
Result Lynx::handle_while(const Self& self, Env& env){}
Result Lynx::eval_atom(const Self& self, Env& env){}
Result Lynx::eval_list(const Self& self, Env& env){}

// Constructors
Result Lynx::fn_bool(const Vec<Self>& args){}
Result Lynx::fn_integer(const Vec<Self>& args){}
Result Lynx::fn_float(const Vec<Self>& args){}
Result Lynx::fn_complex(const Vec<Self>& args){}
Result Lynx::fn_string(const Vec<Self>& args){}
Result Lynx::fn_list(const Vec<Self>& args){}

// Predicates
Result Lynx::fn_is_bool(const Vec<Self>& args){}
Result Lynx::fn_is_number(const Vec<Self>& args){}
Result Lynx::fn_is_integer(const Vec<Self>& args){}
Result Lynx::fn_is_float(const Vec<Self>& args){}
Result Lynx::fn_is_complex(const Vec<Self>& args){}
Result Lynx::fn_is_string(const Vec<Self>& args){}
Result Lynx::fn_is_list(const Vec<Self>& args){}
Result Lynx::fn_is_callable(const Vec<Self>& args){}
Result Lynx::fn_is_builtin(const Vec<Self>& args){}
Result Lynx::fn_is_closure(const Vec<Self>& args){}
Result Lynx::fn_is_lambda(const Vec<Self>& args){}
Result Lynx::fn_is_function(const Vec<Self>& args){}

// I/O functions
Result Lynx::fn_print(const Vec<Self>& args){}
Result Lynx::fn_eprint(const Vec<Self>& args){}
Result Lynx::fn_println(const Vec<Self>& args){}
Result Lynx::fn_eprintln(const Vec<Self>& args){}
Result Lynx::fn_input(const Vec<Self>& args){}
Result Lynx::fn_format(const Vec<Self>& args){}

// Arithmetic operators
Result Lynx::fn_add(const Vec<Self>& args){}
Result Lynx::fn_sub(const Vec<Self>& args){}
Result Lynx::fn_mul(const Vec<Self>& args){}
Result Lynx::fn_div(const Vec<Self>& args){}
Result Lynx::fn_mod(const Vec<Self>& args){}

// Relational operators
Result Lynx::fn_lt(const Vec<Self>& args){;}
Result Lynx::fn_le(const Vec<Self>& args){}
Result Lynx::fn_gt(const Vec<Self>& args){}
Result Lynx::fn_ge(const Vec<Self>& args){}
Result Lynx::fn_eq(const Vec<Self>& args){}
Result Lynx::fn_ne(const Vec<Self>& args){}

// Logical operators
Result Lynx::fn_and(const Vec<Self>& args){}
Result Lynx::fn_or(const Vec<Self>& args){}
Result Lynx::fn_not(const Vec<Self>& args){}

// Functional APIs
Result Lynx::fn_map(const Vec<Self>& args){}
Result Lynx::fn_zip(const Vec<Self>& args){}
Result Lynx::fn_filter(const Vec<Self>& args){}
Result Lynx::fn_reduce(const Vec<Self>& args){}
Result Lynx::fn_take(const Vec<Self>& args){}
Result Lynx::fn_take_while(const Vec<Self>& args){}

// Common list & string functions
Result Lynx::fn_len(const Vec<Self>& args){}
Result Lynx::fn_concat(const Vec<Self>& args){}

// Functions on list
Result Lynx::fn_list_head(const Vec<Self>& args){}
Result Lynx::fn_list_tail(const Vec<Self>& args){}
Result Lynx::fn_list_nth(const Vec<Self>& args){}
Result Lynx::fn_list_insert(const Vec<Self>& args){}
Result Lynx::fn_list_remove(const Vec<Self>& args){}
Result Lynx::fn_list_push(const Vec<Self>& args){}
Result Lynx::fn_list_pop(const Vec<Self>& args){}
Result Lynx::fn_list_append(const Vec<Self>& args){}
Result Lynx::fn_list_set(const Vec<Self>& args){}

// Functions on string
Result Lynx::fn_str_capitalize(const Vec<Self>& args){}
Result Lynx::fn_str_upper(const Vec<Self>& args){}
Result Lynx::fn_str_lower(const Vec<Self>& args){}
Result Lynx::fn_str_contains(const Vec<Self>& args){}
Result Lynx::fn_str_find(const Vec<Self>& args){}
Result Lynx::fn_str_split(const Vec<Self>& args){}
Result Lynx::fn_str_join(const Vec<Self>& args){}
Result Lynx::fn_str_replace(const Vec<Self>& args){}
Result Lynx::fn_str_substr(const Vec<Self>& args){}
Result Lynx::fn_str_ltrim(const Vec<Self>& args){}
Result Lynx::fn_str_rtrim(const Vec<Self>& args){}
Result Lynx::fn_str_trim(const Vec<Self>& args){}
Result Lynx::fn_str_startswith(const Vec<Self>& args){}
Result Lynx::fn_str_endswith(const Vec<Self>& args){}

// Mathetical functions
Result Lynx::fn_abs(const Vec<Self>& args){}
Result Lynx::fn_min(const Vec<Self>& args){}
Result Lynx::fn_max(const Vec<Self>& args){}
Result Lynx::fn_ceil(const Vec<Self>& args){}
Result Lynx::fn_floor(const Vec<Self>& args){}
Result Lynx::fn_round(const Vec<Self>& args){}
Result Lynx::fn_log(const Vec<Self>& args){}
Result Lynx::fn_log2(const Vec<Self>& args){}
Result Lynx::fn_log10(const Vec<Self>& args){}
Result Lynx::fn_log1p(const Vec<Self>& args){}
Result Lynx::fn_exp(const Vec<Self>& args){}
Result Lynx::fn_exp2(const Vec<Self>& args){}
Result Lynx::fn_expm1(const Vec<Self>& args){}
Result Lynx::fn_pow(const Vec<Self>& args){}
Result Lynx::fn_sqrt(const Vec<Self>& args){}
Result Lynx::fn_cbrt(const Vec<Self>& args){}
Result Lynx::fn_sin(const Vec<Self>& args){}
Result Lynx::fn_cos(const Vec<Self>& args){}
Result Lynx::fn_tan(const Vec<Self>& args){}
Result Lynx::fn_asin(const Vec<Self>& args){}
Result Lynx::fn_acos(const Vec<Self>& args){}
Result Lynx::fn_atan(const Vec<Self>& args){}
Result Lynx::fn_atan2(const Vec<Self>& args){}
Result Lynx::fn_erf(const Vec<Self>& args){}
Result Lynx::fn_erfc(const Vec<Self>& args){}
Result Lynx::fn_tgamma(const Vec<Self>& args){}
Result Lynx::fn_lgamma(const Vec<Self>& args){}
Result Lynx::fn_isfinite(const Vec<Self>& args){}
Result Lynx::fn_isinf(const Vec<Self>& args){}
Result Lynx::fn_isnan(const Vec<Self>& args){}

// Miscellaneous functions
Result Lynx::fn_random(const Vec<Self>& args){}
Result Lynx::fn_range(const Vec<Self>& args){}
Result Lynx::fn_linspace(const Vec<Self>& args){}
Result Lynx::fn_sort(const Vec<Self>& args){}
Result Lynx::fn_now(const Vec<Self>& args){}
Result Lynx::fn_today(const Vec<Self>& args){}
Result Lynx::fn_sleep(const Vec<Self>& args){}
Result Lynx::fn_timeit(const Vec<Self>& args){}
Result Lynx::fn_eval(const Vec<Self>& args){}
Result Lynx::fn_declare_module(const Vec<Self>& args){}
Result Lynx::fn_help(const Vec<Self>& args){}
*/

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-