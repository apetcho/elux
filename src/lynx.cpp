#include "lynx.hpp"

#include<iostream>
#include<stack>

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
Env::Env(Env *env) noexcept
: m_bindings{}
, m_parent{env}
{}

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
    // if(!this->contains(key)){
    //     std::stringstream ss;
    //     ss << "unbound identifier `" << key << "'. Cannot be updated.";
    //     throw Error(Error::Kind::RuntimeError, ss.str());
    // }
    if(this->m_bindings.find(key) != this->m_bindings.end()){
        this->m_bindings[key] = val;
        return share();
    }
    auto self = this->m_bindings[key];
    this->m_bindings[key] = val;

    // return this->m_parent->update(key, val);
    return std::move(self);
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

/*
Vec<Str> Env::keys(void) const{}
Vec<std::pair<Str, Self>> Env::items(void) const{}
*/

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
Module::Module(const Str& name, const Dict& dict, Env* env) noexcept
: m_name{name}
, m_path{fs::path("@lynx")} // for builtin modules
, m_env{Env(env)}{
    this->initialize(dict);
}

// -*-
Module::Module(const Str& name, const fs::path& path, Env* env) noexcept
: m_name{name}
, m_path{path}
, m_env{Env(env)}{
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

// -*-
const fs::path& Module::path(void) const{
    return this->m_path;
}

// -*-
const Env& Module::env(void) const{
    return this->m_env;
}

void Module::initialize(void){
    Vec<Str> args{};
    auto filename = this->m_path.string();
    Lynx::run(filename, args, this->m_env);    
}

// -*-
void Module::initialize(const Dict& dict){
    for(const auto& [key, val]: dict){
        this->m_env.put(key, val);
    }
}

// -----------------------------
// -*- Lynx: the interpreter -*-
// -----------------------------
Env Lynx::lynxDocs;
Env Lynx::docstrs;
Env Lynx::prelude;
std::map<Str, Module> Lynx::libraries;
Env Lynx::m_runtime;
std::map<Str, Module> Lynx::m_imported_libs;

// -*-
const Env& Lynx::runtime(void) const{
    return this->m_runtime;
}

// -*-
Env& Lynx::runtime(void){
    return this->m_runtime;
}

static Str _my_highlight(const Str& str){
    Str msg{"\x1b[91m"};
    msg += str;
    msg += "\x1b[m";
    return msg;
};

// -*-
void Lynx::repl(Env& env){
    Env ctx(&env);
    i64 id = 0;
    auto _lynx_prompt = [](i64& idx){
        idx++;
        std::cout << "\x1b[92mlynx::\x1b[93m" << idx << "\x1b[m>> ";
    };

    auto highlight = [](const Str& str){
        Str msg{"\x1b[91m"};
        msg += str;
        msg += "\x1b[m";
        return msg;
    };
    auto checkError = [](const Result& result){
        if(result.is_ok()){
            return false;
        }
        auto msg = result.err().describe();
        auto pos = msg.find(":");
        Str prefix{};
        if(pos != Str::npos){
            prefix = _my_highlight(msg.substr(0, pos));
            msg = msg.substr(pos);
        }
        std::cerr << prefix << "\n" << msg << std::endl;
        return true;
    };
    //! @todo implement help() :: {:?, :h, :help }
    //! @todo implement show() :: {:s, :show }
    //! @todo implement quit() :: {:q, :quit, :exit }
    while(true){
        _lynx_prompt(id);
        auto src = input();
        if(src==":q" || src == ":quit" || src==":bye"){
            std::exit(EXIT_SUCCESS);
        }
        std::istringstream stream(src);
        Parser parser(std::move(stream));
        auto result = parser.parse();
        if(!checkError(result)){
            auto self = result.ok();
            result = Lynx::eval(self, ctx);
            if(!checkError(result)){
                self = result.ok();
                std::cout << self->str() << std::endl;
            }
        }
    }
}

// -*-
void Lynx::run(const Str& filename, const Vec<Str>& args, Env& env){
    Env ctx(&env);

    auto checkError = [](const Result& result){
        if(result.is_ok()){
            return false;
        }
        auto msg = result.err().describe();
        auto pos = msg.find(":");
        Str prefix{};
        if(pos != Str::npos){
            prefix = _my_highlight(msg.substr(0, pos));
            msg = msg.substr(pos);
        }
        std::cerr << prefix << "\n" << msg << std::endl;
        return true;
    };

    Vec<Self> vec{};
    for(const auto arg: args){
        std::istringstream stream(arg);
        Parser parser(std::move(stream));
        auto result = parser.parse();
        if(!checkError(result)){
            auto self = result.ok();
            result = Lynx::eval(self, ctx);
            if(!checkError(result)){
                self = result.ok();
                vec.push_back(self);
            }else{
                std::exit(EXIT_FAILURE);
            }
        }else{
            std::exit(EXIT_FAILURE);
        }
    }
    ctx.update("ARGV", share(vec));
    vec = {};

    // auto src = Lynx::readfile(filename);
    std::ifstream fin(filename);
    Parser parser(std::move(fin));
    auto result = parser.parse();
    if(!checkError(result)){
        auto self = result.ok();
        while(self->type()!=Symbol("nil")){
            vec.push_back(self);
            result = parser.parse();
            if(!checkError(result)){
                self = result.ok();
            }else{
                std::exit(EXIT_FAILURE);
            }
        }
    }else{
        std::exit(EXIT_FAILURE);
    }

    // -*- Evaluation
    Self self = nullptr;
    for(const auto expr: vec){
        result = Lynx::eval(expr, ctx);
        if(!checkError(result)){
            self = result.ok();
        }else{
            std::exit(EXIT_FAILURE);
        }
    }

    if(self != nullptr){
        std::cout << self->str() << std::endl;
    }
}

// -*-
void Lynx::setup(void){
    Lynx::initialize_prelude();
    Lynx::initialize_math_module();
}

// -*-
Result Lynx::eval(const Self& self, Env& env){
    if(Lynx::is_atom(self)){
        return Lynx::eval_atom(self, env);
    }else if(self->is_list()){
        auto xs = *dynamic_cast<List*>(self.get());
        if(xs.len()==0){
            //! @todo: Yield an error instead as this is normally a function application.
            return Result(share(xs.as_list()));
        }
        auto vec = xs.as_vector();
        auto term = vec[0];
        auto args = Vec<Self>(vec.begin()+1, vec.end());

        auto term_ = Lynx::eval(vec[0], env);
        if(!term_.is_ok()){
            return term_;
        }
        if(!term_.ok()->is_lambda() && term->type()!=Symbol("symbol")){
            std::stringstream ss;
            ss << "unknown identifier `" << self->str() << "'\n";
            ss << "Expect a lambda expression or a valid identifier or keyword.";
            Error err(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(err));
        }
        
        if(term_.ok()->is_lambda()){
            return Lynx::eval_list(self, env);
        }
        // We certain that `term' is a symbol.
        auto ident = *dynamic_cast<Symbol*>(term.get());
        
        auto name = ident.str();
        if(Lynx::is_keyword(ident.str())){
            // (cond ...) => args = List(...)
            // ... == xs ==> args = List(xs)
            if(name=="cond"){ return Lynx::handle_cond(share(args), env); }
            if(name=="defvar"){ return Lynx::handle_defvar(share(args), env); }
            if(name=="for"){ return Lynx::handle_for(share(args), env); }
            if(name=="fun"){ return Lynx::handle_fun(share(args), env); }
            if(name=="if"){ return Lynx::handle_if(share(args), env); }
            if(name=="import"){ return Lynx::handle_import(share(args), env); }
            if(name=="lambda"){ return Lynx::handle_lambda(share(args), env); }
            if(name=="let"){ return Lynx::handle_let(share(args), env); }
            if(name=="macro"){ return Lynx::handle_macro(share(args), env); }
            if(name=="progn"){ return Lynx::handle_progn(share(args), env); }
            // [quasi]quote may contains `unquote' and `unquote-splicing'
            if(name=="quote"){ return Lynx::handle_quote(share(args), env); }
            if(name=="quasiquote"){return Lynx::handle_quasiquote(share(args), env); }
            // if(name=="unquote"){ return Lynx::handle_unquote(share(args), env); }
            // if(name=="unquote-splicing"){ return Lynx::handle_unquote_splicing(share(args), env); }
            if(name=="match"){ return Lynx::handle_match(share(args), env); }
            if(name=="var"){ return Lynx::handle_var(share(args), env); }
            if(name=="while"){ return Lynx::handle_while(share(args), env); }
        }else{ // function call
            // // ident is either a builtin funtion or user-defined lambda or function
            // if(!env.contains(name)){
            //     std::stringstream ss;
            //     ss << "undefined function `" << name << "' in scope";
            //     Error err(Error::Kind::RuntimeError, ss.str());
            //     return Result(std::move(err));
            // }
            // auto obj = env.get(name);
            // if(!obj->is_callable()){
            //     std::stringstream ss;
            //     ss << "`" << name << "' is not a callable object.";
            //     Error err(Error::Kind::TypeError, ss.str());
            //     return Result(std::move(err));
            // }
            // Vec<Self> argv{};
            // for(const auto& arg: args){
            //     auto arg_ = Lynx::eval(arg, env);
            //     if(!arg_.is_ok()){
            //         return arg_;
            //     }
            //     argv.push_back(std::move(arg_.ok()));
            // }
            // if(obj->is_builtin()){
            //     auto fun = *dynamic_cast<Builtin*>(obj.get());
            //     return fun(argv);
            // }else if(obj->is_closure()){
            //     auto fun = *dynamic_cast<Closure*>(obj.get());
            //     return fun(argv);
            // }else{
            //     auto macro = *dynamic_cast<Closure*>(obj.get());
            //     return macro(argv);
            // }
            return Lynx::eval_list(self, env);
        }
    }

    // -*-
    Error error("unexpected occured during evaluation.");

    return Result(std::move(error));
}

// -*-
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

// -*-
Str Lynx::input(void){
    std::stack<char> parens{};
    char c{};
    std::cin >> c;
    Str src{};
    Str indent(4, ' ');
    if(c=='('){
        parens.push(c);
        std::cin.unget();
        bool first = true;
        while(!parens.empty()){
            if(first){
                parens.pop();
                first = false;
            }
            Str line{};
            std::getline(std::cin >> std::ws, line);
            src += line + "\n";
            for(int i=0; i < line.length(); i++){
                c = line[i];
                if(c == ')'){ parens.pop(); }
                else if(c=='('){ parens.push(c); }
            }
            auto level = parens.size();
            if(level != 0){
                // if(!first){ std::cout << "...."; }
                for(int k=0; k < level; k++){
                    std::cout << indent;
                }
            }
        }
    }else{
        std::cin.unget();
        std::getline(std::cin >> std::ws, src);
    }
    return src;
}

// -*-
Str Lynx::make_library_key(const Module& mymodule){
    auto name = mymodule.name().str();
    auto path = mymodule.path().string();
    auto key = name + "::";
    key += path;
    return key;
}

// -*-
void Lynx::push_module(const Module& mymodule){
    auto key = Lynx::make_library_key(mymodule);
    Lynx::libraries.insert({key, mymodule});
}

// -*-
void Lynx::import_module(const Str& name, Env& env){
    if(Lynx::m_imported_libs.find(name) != Lynx::m_imported_libs.end()){
        return;
    }
    bool found = false;
    for(const auto& [key, _]: Lynx::libraries){
        if(String(key).startswith(String(name))){
            found = true;
            break;
        }
    }
    if(!found){
        std::stringstream ss;
        ss << "module '" << name << "' not found.";
        throw Error(Error::Kind::RuntimeError, ss.str());
    }
    auto entry = Lynx::libraries.find(name);
    auto lib = entry->second;
    auto _name_ = Lynx::make_library_key(lib);
    Lynx::m_imported_libs.insert({_name_, lib});
    auto _env = lib.env();
    auto keys = _env.keys();
    for(const auto& key: keys){
        env.m_bindings.insert({key, _env.get(key)});
    }
}

// -*-
//! @todo Refactor
void Lynx::import_module(const fs::path& module_path, Env& env){
    // -*-
    if(!fs::exists(module_path)){
        std::stringstream ss;
        ss << "module '" << module_path.string() << "' not found";
        throw Error(Error::Kind::RuntimeError, ss.str());
    }
    auto name = module_path.stem();
    Module mymodule(name, module_path, &Lynx::m_runtime);
    auto key = Lynx::make_library_key(mymodule);
    Lynx::m_imported_libs.insert({key, mymodule});
    auto _env = mymodule.env();
    for(const auto& key: _env.keys()){
        env.m_bindings.insert({key, _env.get(key)});
    }
}

// -*-
bool Lynx::check_argc(int argc, int expected, const Str& funcname, Error& err){
    if(argc==expected){ return true; }
    std::stringstream ss;
    if(funcname.length()!=0){
        ss << "`" << funcname << "': ";
    }
    ss << "invalid number of arguments. Expected " << expected << ", got " << argc;
    err = Error(Error::Kind::SyntaxError, ss.str());
    return false;
}

// -*-
bool Lynx::check_type(const Symbol& ty, const Self& self, Error& err){
    if(ty==self->type()){ return true; }
    std::stringstream ss;
    ss << "type mismatch. Expected `" << ty.str() << "', got `";
    ss << self->type().str() << "'";
    err = Error(Error::Kind::TypeError, ss.str());
    return false;
}

// -*-
bool Lynx::check_value(const Self& self, bool (*fn)(const Self&), Error& err){
    if(fn(self)){ return true; }
    std::stringstream ss;
    ss << "unexpected value `" << self->str() << "'";
    err = Error(Error::Kind::ValueError, ss.str());
    return false;
}

// -*-
bool Lynx::check_value(const Self& self, bool pred, Error& err){
    if(pred){ return true; }
    std::stringstream ss;
    ss << "unexpected value `" << self->str() << "'";
    err = Error(Error::Kind::ValueError, ss.str());
    return false;
}

/** @todo
bool Lynx::check_argc(bool pred, const Str& funcname, Error& err){}
bool Lynx::check_type(bool pred, const Self& self, Error& err){}
*/
// -*-
bool Lynx::is_reserved_word(const Str& word){
    static std::map<Str, TokenKind> _reserved_words_ = {
#define LYNX_DEF(tok, name)     {name, TokenKind::tok},
        LYNX_KEYWORDS()
#undef LYNX_DEF
        {"nil", TokenKind::Nil},
        {"true", TokenKind::True},
        {"false", TokenKind::False},
    };
    auto entry = _reserved_words_.find(word);
    if(entry==_reserved_words_.end()){
        return false;
    }
    return true;
}

// -*-
bool Lynx::is_keyword(const Str& word){
        static std::map<Str, TokenKind> _reserved_words_ = {
#define LYNX_DEF(tok, name)     {name, TokenKind::tok},
        LYNX_KEYWORDS()
#undef LYNX_DEF
    };
    auto entry = _reserved_words_.find(word);
    if(entry==_reserved_words_.end()){
        return false;
    }
    return true;
}

// -*-
bool Lynx::is_syntax_quote(const Self& self){
    return (
        self->is_symbol() && (
            self->str() == "quote" ||
            self->str() == "unquote" ||
            self->str() == "quasiquote" ||
            self->str() == "unquote-splicing"
        )
    );
}

// -*-
Vec<Symbol> Lynx::get_symbols(const Vec<Self>& body){
    Vec<Symbol> result{};
    for(const auto self: body){
        if(self->is_symbol()){
            auto sym = *dynamic_cast<Symbol*>(self.get());
            if(!Lynx::is_reserved_word(sym.str())){
                result.push_back(sym);
            }
        }
    }

    return result;
}

// -*-
bool Lynx::match(const Symbol& sym, const Self& self){
    if(self->is_symbol()){
        auto _sym_ = *dynamic_cast<Symbol*>(self.get());
        return sym==_sym_;
    }
    return false;
}

// -*-
void Lynx::expect(const Symbol& sym, const Self& self){
    if(self->is_symbol()){
        auto _sym_ = *dynamic_cast<Symbol*>(self.get());
        if(sym!=_sym_){
            std::stringstream ss;
            ss << "symbols mismatched. Expected `" << sym.str() << "'";
            ss << ", got `" << _sym_.str() << "'";
            throw Error(Error::Kind::ValueError, ss.str());
        }
    }
}

// -*-
bool Lynx::is_atom(const Self& self){
    bool ans{};
    ans = (
        self->is_bool() || self->is_integer() ||
        self->is_float() || self->is_symbol() ||
        self->is_string()
    );

    return ans;
}

// -*-
bool Lynx::to_bool(const Self& self){
    if(self->is_bool()){
        auto val = *dynamic_cast<Bool*>(self.get());
        return val.as_bool();
    }else if(self->is_integer()){
        auto val = *dynamic_cast<Number*>(self.get());
        return val.as_integer() == 0 ? false : true;
    }else if(self->is_float()){
        auto val = *dynamic_cast<Number*>(self.get());
        return val.as_float() == 0.0 ? false : true;
    }else if(self->type()==Symbol("nil")){
        return false;
    }
    return true;
}


// -*-
Result Lynx::handle_cond(const Self& self, Env& env){
    //! @todo: add doc-string of `cond' to lynxDocs describing it syntax
    /*
        (cond
            (pred-1 expr1)
            (pred-2 expr2)
            ...
            (pred-N exprN))

    Pre: self is a list of pairs (pred expr)
         matched = false
    Post:
        matched = true if at least one of the predicate evaluates to true
                otherwise throw a SyntaxError
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = dynamic_cast<List*>(self.get());
    auto vec = xs->as_vector();
    for(const auto clause: vec){
        if(!Lynx::check_type(Symbol("list"), clause, err)){
            return Result(std::move(err));
        }

        auto _xs_ = *dynamic_cast<List*>(clause.get());
        if(!Lynx::check_argc(_xs_.len(), 2, "", err)){
            return Result(std::move(err));
        }
        auto _vec_ = _xs_.as_vector();
        auto ans = Lynx::eval(_vec_[0], env);
        if(!ans.is_ok()){
            err = ans.err();
            return Result(std::move(err));            
        }
        auto val = ans.ok();
        if(Lynx::to_bool(val)){
            return Lynx::eval(_vec_[1], env);
        }
    }

    err = Error(Error::Kind::SyntaxError, "umatched clause in `cond' special form.");
    return Result(std::move(err));
}

// -*-
Result Lynx::handle_defvar(const Self& self, Env& env){
    //! @todo: add doc-string of `defvar' to lynxDocs describing it syntax
    /*
        (defvar name value)
        (defvar name value docstr)
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()==2 || xs.len()==3);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `defvar'. Takes 2 or 3 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(!Lynx::check_type(Symbol("symbol"), vec[0], err)){
        return Result(std::move(err));
    }
    Str _doc_{};
    auto sym = *dynamic_cast<Symbol*>(vec[0].get());
    auto name = sym.str();
    auto val = vec[1];
    val->immutable() = true;
    if(xs.len()==2){
        _doc_ = "";
    }else{
        if(!Lynx::check_type(Symbol("string"), vec[2], err)){
            return Result(std::move(err));
        }
        auto msg = *dynamic_cast<String*>(vec[2].get());
        _doc_ = msg.str();
    }
    if(env.contains(name)){
        Str msg{"cannot redifined immutable variable `'"};
        msg += name + "'";
        err = Error(Error::Kind::RuntimeError, msg);
        return Result(std::move(err));
    }else{
        env.put(name, val);
    }

    if(Lynx::docstrs.contains(name)){
        Lynx::docstrs.update(name, share(_doc_));    
    }else{
        Lynx::docstrs.put(name, share(_doc_));
    }

    return Result(share());
}

// -*-
Result Lynx::handle_for(const Self& self, Env& env){
    //! @todo: add doc-string of `for' to lynxDocs describing it syntax
    /*
        (for (x xs) body)

        Pre:
            x is a symbol
            xs evaluate to a list
            body is any valid expression


        Example:
            (for (x '(1 2 3 4))
                (println (format "x = {x}")))

            >>
            x = 1
            x = 2
            x = 3
            x = 4
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()>=1);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `for'. Takes at least 1 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(!Lynx::check_type(Symbol("list"), vec[0], err)){
        return Result(std::move(err));
    }
    auto _iter_ = vec[0]; // iteration clause
    auto _xxs_ = *dynamic_cast<List*>(_iter_.get());
    auto body = Vec<Self>(vec.begin()+1, vec.end()); // the body of the loop
    pred = (_xxs_.len() == 2);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `for'. The first argument to `for' special form must be ";
        ss << "a list of 2 elements.";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    auto _xxsvec_ = _xxs_.as_vector();
    auto _x_ = _xxsvec_[0];     // iteration variable
    auto _xs_ = _xxsvec_[1];    // iterable
    
    if(!Lynx::check_type(Symbol("symbol"), _x_, err)){
        return Result(std::move(err));
    }
    auto _xs = Lynx::eval(_xs_, env);
    if(!_xs.is_ok()){
        return Result(std::move(_xs.err()));
    }
    auto xs_ = _xs.ok();
    auto iterable = *dynamic_cast<List*>(xs_.get());
    auto values = iterable.as_vector();
    auto _sym_ = *dynamic_cast<Symbol*>(_x_.get());
    auto name = _sym_.str();
    // Create the loop's environment
    Env ctx(&env);
    // initialize the loop variable
    Self ans = nullptr; // for storing the final result
    ctx.put(name, share());
    for(const auto& value: values){
        // evaluate each loop variable
        auto val = Lynx::eval(value, ctx);
        if(!val.is_ok()){
            return Result(std::move(val.err()));
        }
        // add the evaluated value to the loop environment
        ctx.update(name, val.ok());
        // evalue the loop's body
        for(const auto& expr: body){
            auto _ans = Lynx::eval(expr, ctx);
            if(!_ans.is_ok()){
                return Result(std::move(_ans.err()));
            }
            ans = _ans.ok();
        }
    }
    
    return Result(std::move(ans));
}

// -*-
Result Lynx::handle_fun(const Self& self, Env& env){
    //! @todo: add doc-string of `fun' to lynxDocs describing it syntax 
    /*
        (fun name params [docstr] body)

        Examples
        (1) Without docstr
            (fun favorite-programming-languages ()
                (println "(1) C programming language")
                (println "(2) C++ programming language")
                (println "(3) Rust programming language"))

        (2) With a docstr
            (fun quadratic-equation-solver (a b c)
                "Solve the quadratic a equation for real numbers."
                (var discriminant (- (pow b 2) (* 4 a c)))
                (progn
                    (cond
                        ((> discriminant 0)
                            (progn
                                (var result '())
                                (var delta (sqrt discriminant))
                                (var x1 (/ (- (- b) delta) 2)
                                (var x2 (/ (+ (- b) delta) 2)
                                (push x1 result)
                                (push x2 result)
                                result))
                        ((= discriminant 0)
                            (var x (/ (- b) / 2))
                            (var result (list x))
                            result)
                        ((< discriminant 0)
                            (eprintln "No solution found")))))
    */
    
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()>=2);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `fun' definition. Takes at least 2 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(!Lynx::check_type(Symbol("symbol"), vec[0], err)){
        return Result(std::move(err));
    }
    auto _name = *dynamic_cast<Symbol*>(vec[0].get());
    auto name = _name.str();
    if(!Lynx::check_type(Symbol("list"), vec[1], err)){
        return Result(std::move(err));
    }
    auto _params = *dynamic_cast<List*>(vec[1].get());
    auto params = _params.as_vector();
    for(const auto& param: params){
        if(!Lynx::check_type(Symbol("symbol"), param, err)){
            err = Error(Error::Kind::SyntaxError, "parameter to function must be a symbol");
            return Result(std::move(err));
        }
    }
    // check for duplicates parameters
    std::set<Str> _params_set_{};
    for(const auto param: params){
        auto key = *dynamic_cast<Symbol*>(param.get());
        _params_set_.insert(key.str());
    }
    if(_params_set_.size() != params.size()){
        // there have been duplicate parameters
        err = Error(Error::Kind::SyntaxError, "duplicate parameters in function definition");
        return Result(std::move(err));
    }
    Vec<Self> body{};
    Str _doc_{};
    if(vec[2]->is_string()){
        auto _my_doc = *dynamic_cast<String*>(vec[2].get());
        _doc_ = _my_doc.str();
        body = Vec<Self>(vec.begin()+3, vec.end());
    }else{
        body = Vec<Self>(vec.begin()+2, vec.end());
    }
    auto _symbols_ = Lynx::get_symbols(body);
    Env ctx;

    for(const auto& sym: _symbols_){
        // check whether `sym' belong to the parent environment and 'capture' it so.
        if(_params_set_.find(sym.str())==_params_set_.end()){
            // is `sym' actually defined
            if(!env.contains(sym.str())){
                Str msg{"unbound variable `"};
                msg += sym.str() + "'";
                err = Error(Error::Kind::RuntimeError, msg);
                return Result(std::move(err));
            }
            ctx.put(sym.str(), env.get(sym.str()));
        }
    }
    
    auto _my_params_ = Vec<Symbol>(_params_set_.begin(), _params_set_.end());
    if(Lynx::docstrs.contains(name)){
        Lynx::docstrs.update(name, share(_doc_));
    }else{
        Lynx::docstrs.put(name, share(_doc_));
    }

    return Result(share(name, _my_params_, body, ctx));
}

// -*-
Result Lynx::handle_if(const Self& self, Env& env){
    //! @todo: add doc-string of `if' to lynxDocs describing it syntax
    /*
        [1] (if test-expr ok-body)
        [2] (if test-expr ok-body no-body)
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    

    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()==2 || xs.len()==3);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `if' expression. Takes at least 2 or 3 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    auto vec = xs.as_vector();
    auto testExpr = vec[0];
    auto testResult = Lynx::eval(testExpr, env);
    if(!testResult.is_ok()){
        return testResult;
    }
    auto test = testResult.ok();
    if(!test->is_bool()){
        std::stringstream ss;
        ss << "malformed `if' expression. Test-expression must evaluate to boolean";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    if(test){// test evaluate to true. Take the ok-branch
        return Lynx::eval(vec[1], env);
    }
    // Test evaluate to false. If no-banch exist, evaluate it.
    if(xs.len() == 3){
        return Lynx::eval(vec[2], env);
    }
    // no-branch does exist, return nil
    return Result(share());
}

// -*-
Result Lynx::handle_import(const Self& self, Env& env){
    //! @todo: add doc-string of `import' to lynxDocs describing it syntax
    /*
        (import module-name)
        (import "/path/to/module.lynx")
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    

    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()==1);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `import' expression. Takes 1 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    auto vec = xs.as_vector();
    auto _mymodule = vec[0];
    pred = (_mymodule->is_string() || _mymodule->is_symbol());
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `import' expression. Argument must be a string or symbol";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    [[maybe_unused]] bool found = false;
    Str name{};
    if(_mymodule->is_symbol()){
        auto sym = *dynamic_cast<Symbol*>(_mymodule.get());
        if(Lynx::is_imported(sym)){ // already imported
            return Result(share());
        }
        // module not yet imported. Import it now.
        name = sym.str();
        try{
            Lynx::import_module(name, env);
            found = true;
        }catch(const Error& __err__){
            auto _Err = __err__;
            return Result(std::move(_Err));
        }
    }else{
        auto _str_ = *dynamic_cast<String*>(_mymodule.get());
        auto _mypath = fs::path(_str_.str());
        name = _mypath.stem();
        try{
            Lynx::import_module(_mypath, env);
            found = true;
        }catch(const Error& __err__){
            auto _Err = __err__;
            return Result(std::move(_Err));
        }
        found = true;
    }

    if(!found){
        std::stringstream ss;
        ss << "module `" << name << "' not found.";
        err = Error(Error::Kind::RuntimeError, ss.str());
        return Result(std::move(err));
    }
    
    return Result(share());
}

// -*-
Result Lynx::handle_lambda(const Self& self, Env& env){
    //! @todo: add doc-string of `lambda' to lynxDocs describing it syntax
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()>=1);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `lambda' definition. Takes at least 1 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    
    auto _params = *dynamic_cast<List*>(vec[0].get());
    auto params = _params.as_vector();
    for(const auto& param: params){
        if(!Lynx::check_type(Symbol("symbol"), param, err)){
            err = Error(Error::Kind::SyntaxError, "parameter to function must be a symbol");
            return Result(std::move(err));
        }
    }
    // check for duplicates parameters
    std::set<Str> _params_set_{};
    for(const auto param: params){
        auto key = *dynamic_cast<Symbol*>(param.get());
        _params_set_.insert(key.str());
    }
    if(_params_set_.size() != params.size()){
        // there have been duplicate parameters
        err = Error(Error::Kind::SyntaxError, "duplicate parameters in function definition");
        return Result(std::move(err));
    }
    Vec<Self> body{};
    body = Vec<Self>(vec.begin()+1, vec.end());
    auto _symbols_ = Lynx::get_symbols(body);
    Env ctx;

    for(const auto& sym: _symbols_){
        // check whether `sym' belong to the parent environment and 'capture' it so.
        if(_params_set_.find(sym.str())==_params_set_.end()){
            // is `sym' actually defined
            if(!env.contains(sym.str())){
                Str msg{"unbound variable `"};
                msg += sym.str() + "'";
                err = Error(Error::Kind::RuntimeError, msg);
                return Result(std::move(err));
            }
            ctx.put(sym.str(), env.get(sym.str()));
        }
    }
    
    auto _my_params_ = Vec<Symbol>(_params_set_.begin(), _params_set_.end());

    return Result(share(_my_params_, body, ctx));
}

// -*-
Result Lynx::handle_let(const Self& self, Env& env){
    //! @todo: add doc-string of `import' to lynxDocs describing it syntax
    /*
        (let pairs body)

    Examples:
        (let ((x 1)
              (y 2))
            (+ x y))
    */

    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()==2);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `let' expression. Takes 2 arguments";
        ss << ", got " << xs.len();
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(!Lynx::check_type(Symbol("list"), vec[0], err)){
        return Result(std::move(err));
    }
    Env ctx(&env);
    auto _pairs = *dynamic_cast<List*>(vec[0].get());
    auto pairs = _pairs.as_vector();
    for(const auto& _pair: pairs){
        if(!Lynx::check_type(Symbol("list"), _pair, err)){
            return Result(std::move(err));
        }
        auto __pair = *dynamic_cast<List*>(_pair.get());
        pred = (__pair.len()==2);
        if(!Lynx::check_value(self, pred, _err_)){
            std::stringstream ss;
            ss << "malformed `let' expression. Expect the first arguments to be ";
            ss << "a list of pairs ";
            err = Error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(err));
        }
        auto pair = __pair.as_vector();
        if(!Lynx::check_type(Symbol("symbol"), pair[0], err)){
            return Result(std::move(err));
        }
        auto _name = *dynamic_cast<Symbol*>(pair[0].get());
        ctx.put(_name.str(), pair[1]);
    }
    auto body = vec[1];
    return Lynx::eval(body, ctx);
}

// -*-
Result Lynx::handle_macro(const Self& self, Env& env){
    /** @note: 
    Hygynic macro should not capture variable from the outer environment
    during definition phase.
    */
    //! @todo: add doc-string of `import' to lynxDocs describing it syntax
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()>=2);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `macro' definition. Takes at least 2 arguments";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(!Lynx::check_type(Symbol("symbol"), vec[0], err)){
        return Result(std::move(err));
    }
    auto _name = *dynamic_cast<Symbol*>(vec[0].get());
    auto name = _name.str();
    if(!Lynx::check_type(Symbol("list"), vec[1], err)){
        return Result(std::move(err));
    }
    auto _params = *dynamic_cast<List*>(vec[1].get());
    auto params = _params.as_vector();
    for(const auto& param: params){
        if(!Lynx::check_type(Symbol("symbol"), param, err)){
            err = Error(Error::Kind::SyntaxError, "parameter to function must be a symbol");
            return Result(std::move(err));
        }
    }
    // check for duplicates parameters
    std::set<Str> _params_set_{};
    for(const auto param: params){
        auto key = *dynamic_cast<Symbol*>(param.get());
        _params_set_.insert(key.str());
    }
    if(_params_set_.size() != params.size()){
        // there have been duplicate parameters
        err = Error(Error::Kind::SyntaxError, "duplicate parameters in function definition");
        return Result(std::move(err));
    }
    Vec<Self> body{};
    Str _doc_{};
    if(vec[2]->is_string()){
        auto _my_doc = *dynamic_cast<String*>(vec[2].get());
        _doc_ = _my_doc.str();
        body = Vec<Self>(vec.begin()+3, vec.end());
    }else{
        body = Vec<Self>(vec.begin()+2, vec.end());
    }
    auto _symbols_ = Lynx::get_symbols(body);
    Env ctx;

    for(const auto& sym: _symbols_){
        // check whether `sym' belong to the parent environment and 'capture' it so.
        if(_params_set_.find(sym.str())==_params_set_.end()){
            // is `sym' actually defined
            if(!env.contains(sym.str())){
                Str msg{"unbound variable `"};
                msg += sym.str() + "'";
                err = Error(Error::Kind::RuntimeError, msg);
                return Result(std::move(err));
            }
            ctx.put(sym.str(), env.get(sym.str()));
        }
    }
    
    auto _my_params_ = Vec<Symbol>(_params_set_.begin(), _params_set_.end());
    if(Lynx::docstrs.contains(name)){
        Lynx::docstrs.update(name, share(_doc_));
    }else{
        Lynx::docstrs.put(name, share(_doc_));
    }

    return Result(share(name, _my_params_, body, ctx, true));
}

// -*-
Result Lynx::handle_progn(const Self& self, Env& env){
    //! @todo: add doc-string of `progn' to lynxDocs describing it syntax
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    // progn should have its own environment.
    Env ctx(&env);
    Self result = nullptr;
    for(const auto& expr: vec){
        auto ans = Lynx::eval(expr, ctx);
        if(!ans.is_ok()){
            return ans;
        }
        result = ans.ok();
    }

    return Result(std::move(result));
}

// -*-
Result Lynx::handle_quote(const Self& self, Env& env){
    //! @todo: add doc-string of `quote' to lynxDocs describing it syntax
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    bool pred = (xs.len()==1);
    [[maybe_unused]] Error _err_;
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `quote' definition. Takes 1 argument";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    return Result(std::move(vec[0]));
}

// -*-
Result Lynx::handle_quasiquote(const Self& self, Env& env){
    //! @todo: add doc-string of `cond' to lynxDocs describing it syntax
    /*
        `expr or (quasiquote expr)
        Algorithm:
            [1] expr is atom  => Error
            [2] expr is a list
                foreach term in expr
                    [2.1] if term is atom => atom
                    [2.2] if term is list and term[0] is unquote
                        => eval(term[1] ... term[:last])
                    [2.3] if term is list and term[0] is unquote-splicing
                        => splice(term[1] ... term[:last])
                    [2.4] if term is list and term[0] is quote
                        => (term[1] ... term[:last])
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    bool pred = (xs.len()==1);
    [[maybe_unused]] Error _err_;
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `quasiquote' definition. Takes 1 argument";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    // Self result = nullptr;
    auto expr = vec[0];
    if(!Lynx::check_type(Symbol("list"), expr, err)){
        return Result(std::move(err));
    }
    auto my_vec = *dynamic_cast<List*>(expr.get());
    auto argv = my_vec.as_vector();
    Vec<Self> ans{};
    for(const auto& arg: argv){
        // arg is either an atom or a list
        // Case 1: arg is an atom
        if(Lynx::is_atom(arg)){
            ans.push_back(std::move(arg));
            continue;
        }

        // Case 2: arg is a list
        // first element in list could be a syntax-quote or anything else
        //else{
        auto tmpv = *dynamic_cast<List*>(arg.get());
        auto tmp = tmpv.as_vector();
        // First element is not a syntax-quote
        if(!Lynx::is_syntax_quote(tmp[0])){
            // other builtin symbol or user-defined symbol
            // We preserved the structure
            ans.push_back(share(tmp));
            continue;
        }// else 
        // From here on, we assume that the First element is a syntax-quote
        // tmp = (a-syntax_quote _arg_)
        // Requirement: tmp.size() == 2
        pred = (tmp.size()==2);
        if(!Lynx::check_value(self, pred, _err_)){
            std::stringstream ss;
            ss << "error while processing quasiquote-form.";
            err = Error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(err));
        }

        if(tmp[0]->is_symbol() && tmp[0]->str()=="quote"){
            // other builtin symbol or user-defined symbol
            // We preserved the structure at this point for later evaluation
            ans.push_back(share(tmp));
            continue;
        }// else 
        if(tmp[0]->is_symbol() && tmp[0]->str()=="unquote"){
            // auto my_expr = share(Vec<Self>(tmp.begin()+1, tmp.end()));
            auto my_ans = Lynx::eval(tmp[1], env);
            if(!my_ans.is_ok()){
                std::stringstream ss;
                ss << "malformed argument to unquote in quasiquote expression";
                err = Error(Error::Kind::RuntimeError, ss.str());
                return Result(std::move(err));
            }
            ans.push_back(std::move(my_ans.ok()));
            continue;
        }//else

        pred = (tmp[1]->is_list());
        if(!Lynx::check_value(self, pred, _err_)){
            std::stringstream ss;
            ss << "error while processing unquote-splicing in quasiquote-form.";
            err = Error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(err));
        }
        if(tmp[0]->is_symbol() && tmp[0]->str()=="unquote-splicing"){
            auto my_xs = *dynamic_cast<List*>(tmp[1].get());
            auto my_vec = my_xs.as_vector();
            for(const auto& val: my_vec){
                ans.push_back(std::move(val));
            }
            continue;
        }
        // -
        if(tmp[0]->is_symbol() && tmp[0]->str()=="quasiquote"){
            // everything from with will be quoted.
            auto my_xs = *dynamic_cast<List*>(tmp[1].get());
            auto my_vec = my_xs.as_vector();
            for(const auto& val: my_vec){
                Vec<Self> term{};
                term.push_back(share("quote"));
                term.push_back(std::move(val));
                ans.push_back(share(term));
            }
            continue;
        }
        // else{}
        // }
    }

    return Result(share(ans));
}

// Result Lynx::handle_unquote(const Self& self, Env& env){}
// Result Lynx::handle_unquote_splicing(const Self& self, Env& env){}

// -*-
Result Lynx::handle_match(const Self& self, Env& env){
    //! @todo: add doc-string of `match' to lynxDocs describing it syntax
    /*
        (match clause
            (pattern1 expr1)
            (pattern2 expr2)
            ...
            (patternN exprN))

        The pattern '_' (i.e ANY-PATTERN) matches against any clause.
        At least one pattern should match in the pattern-list, otherwise an
        error is reported. To avoid error, one should make sure one of the
        patterns matches against the match-clause by possible using the so-called
        ANY-PATTERN

        Right now, matchable patterns include boolean, integers, symbol or string.
        This is so because these type of object provide a well defined equality
        operator.
    */

    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();

    [[maybe_unused]] Error _err_;
    bool pred = (xs.len()>1);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `match' expression. Takes more than 1 argument";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    auto clause_ = Lynx::eval(vec[0], env);
    if(!clause_.is_ok()){
        std::stringstream ss;
        ss << "error occured while evaluate the clause of a match-expressin";
        err = Error(Error::Kind::ValueError, ss.str());
        return Result(std::move(err));
    }
    auto clause = clause_.ok();
    auto branches = Vec<Self>(vec.begin()+1, vec.end());

    auto matchable = [](const Self& arg){
        return (
            arg->is_bool() || arg->is_integer() ||
            arg->is_symbol() || arg->is_string()
        );
    };
    bool matched{false};
    for(const auto& branch: branches){
        // we expect every branch has the form: (pattern expr)
        if(!Lynx::check_type(Symbol("list"), branch, err)){
            return Result(std::move(err));
        }

        auto branch_xs = *dynamic_cast<List*>(branch.get());
        pred = (branch_xs.len()==2);
        if(!Lynx::check_value(self, pred, _err_)){
            std::stringstream ss;
            ss << "malformed `match' expression. Branch must have the form: ";
            ss << "(pattern expr).\nWe've got\n" << branch_xs.repr();
            err = Error(Error::Kind::SyntaxError, ss.str());
            return Result(std::move(err));
        }
        auto branch_vec = branch_xs.as_vector();
        auto pattern_ = branch_vec[0];
        auto expr = branch_vec[1];
        auto pattern_ev = Lynx::eval(pattern_, env);
        if(!pattern_ev.is_ok()){
            return pattern_ev; // error occured
        }
        auto pattern = pattern_ev.ok();
        if(!matchable(pattern)){
            std::stringstream ss;
            ss << "invalid match pattern type. Expect a symbol, boolean, integer or ";
            ss << "string value. Got `" << pattern->type().str() << "' value.";
            err = Error(Error::Kind::TypeError, ss.str());
            return Result(std::move(err));
        }
        if(pattern->is_symbol() && pattern->str()=="_"){ // ANY_PATTERN found
            return Lynx::eval(expr, env);
        }
        if(pattern==clause){
            return Lynx::eval(expr, env);
        }
    }
    // There was not match. The match-expression was not well formed.
    std::stringstream ss;
    ss << "non-exhaustive pattern. None of the pattern matched againt the match-clause.";
    err = Error(Error::Kind::SyntaxError, ss.str());
    return Result(std::move(err));
}

// -*-
Result Lynx::handle_var(const Self& self, Env& env){
    //! @todo: add doc-string of `cond' to lynxDocs describing it syntax
    /*
        (var name value)
        (var name1 value1 name2 value2 ...)
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());

    [[maybe_unused]] Error _err_;
    bool pred = (xs.len() % 2 == 0);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `var' expression. Takes 2 or a multiple of 2 arguments.\n";
        ss << "Note: each successive pair consist of an identifier and the associated value.";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    auto vec = xs.as_vector();
    // Add the name-value pair to the current environment, possibly updating
    // existing name.
    for(size_t i=0; i < xs.len()-1; i++){
        auto name = vec[i];
        if(!Lynx::check_type(Symbol("symbol"), name, err)){
            return Result(std::move(err));
        }
        auto expr_ = Lynx::eval(vec[i+1], env);
        if(!expr_.is_ok()){
            std::stringstream ss;
            ss << "invalid expression " << vec[i+1]->repr() << " binding to ";
            ss << name->str();
            err = Error(Error::Kind::ValueError, ss.str());
            return Result(std::move(err));
        }
        auto expr = expr_.ok();
        env.update(name->str(), expr);
    }

    return Result(share());
}

// -*-
Result Lynx::handle_while(const Self& self, Env& env){
    //! @todo: add doc-string of `cond' to lynxDocs describing it syntax
    /*
        (while testExpr
            body)
    */
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());

    [[maybe_unused]] Error _err_;
    bool pred = (xs.len() > 1);
    if(!Lynx::check_value(self, pred, _err_)){
        std::stringstream ss;
        ss << "malformed `while' expression. Takes at least 1 arguments.";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    bool failed{false};
    Env ctx(&env);
    auto evalTest = [ctx, failed, err](const Self& expr) mutable {
        auto testExpr_ = Lynx::eval(expr, ctx);
        if(!testExpr_.is_ok()){
            std::stringstream ss;
            ss << "invalid test-expression in while expression.";
            err = Error(Error::Kind::ValueError, ss.str());
            failed = true;
            return failed;
        }
        auto testExpr = testExpr_.ok();
        auto pred = (testExpr->is_bool());
        if(!Lynx::check_value(testExpr, pred, err)){
            std::stringstream ss;
            ss << "malformed `while' expression. Expect test-expression to evaluate to a boolean.\n";
            ss << "Got `" << testExpr->type().str() << "' value.";
            err = Error(Error::Kind::SyntaxError, ss.str());
            // return Result(std::move(err_));
            failed = true;
            return failed;
        }
        auto test_ = *dynamic_cast<Bool*>(testExpr.get());
        auto test = test_.as_bool();
        failed = false;
        return test;
    };

    auto vec = xs.as_vector();
    auto test = evalTest(vec[0]);
    auto body = Vec<Self>(vec.begin()+1, vec.end());
    Self ans = nullptr;
    if(failed){
        return Result(std::move(err));
    }
    while(test){
        for(const auto& expr: body){
            auto ans_ = Lynx::eval(expr, ctx);
            if(!ans_.is_ok()){ return ans_; }
            ans = ans_.ok();
        }
        test = evalTest(vec[0]);
    }

    return Result(share());
}

// -*-
Result Lynx::eval_atom(const Self& self, [[maybe_unused]] Env& env){
    //! @todo: add doc-string of `cond' to lynxDocs describing it syntax

    [[maybe_unused]] Error err;
    bool pred = Lynx::is_atom(self);
    if(!Lynx::check_value(self, pred, err)){
        std::stringstream ss;
        ss << "error while evaluating an atom. Value is of `" << self->type().str();
        ss << "' type. Expect a boolean, symbol, integer, float or string";
        err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    auto ans = self;
    return Result(std::move(ans));
}

// -*-
Result Lynx::eval_list(const Self& self, Env& env){
    //! @note: this is a function-call or builtin syntax-application
    /*
        (symbol ...)
        (lambda-expr ...)

        symbol is:
        [1] builtin keyword for syntax application
        [2] builtin function name
        [3] user-defined function
    */
    
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();
    auto car_ = Lynx::eval(vec[0], env);
    if(!car_.is_ok()){
        return car_;
    }
    auto car = car_.ok();
    auto args = Vec<Self>(vec.begin()+1, vec.end());
    Vec<Self> argv{};
    for(const auto& arg: args){
        auto arg_ = Lynx::eval(arg, env);
        if(!arg_.is_ok()){
            return arg_;
        }
        argv.push_back(std::move(arg_.ok()));
    }
    if(car->is_closure()){
        auto fun = *dynamic_cast<Closure*>(car.get());
        return fun(argv);
    }
    if(car->is_builtin()){
        auto fun = *dynamic_cast<Builtin*>(car.get());
        return fun(argv);
    }
    if(car->is_macro()){
        auto fun = *dynamic_cast<Macro*>(car.get());
        auto prog = fun.expand(argv);
        if(!prog.is_ok()){
            return prog;
        }
        auto expanded = prog.ok();
        if(Lynx::is_atom(expanded)){
            return Result(std::move(expanded));
        }else{
            return Lynx::handle_progn(expanded, env);
        }
    }
    
    std::stringstream ss;
    ss << "expected error occured while evaluating the expression\n";
    ss << self->repr() << "\n";
    err = Error(Error::Kind::SyntaxError, ss.str());

    return Result(std::move(err));
}

// --------------------------------------------------------------------
// -*-                       Constructors                           -*-
// --------------------------------------------------------------------
Result Lynx::fn_bool(const Vec<Self>& args){
    //! @todo: add doc-string of `bool' to lynxDocs describing it syntax
    Error err;
    auto argc = args.size();
    auto pred = (argc==1);
    if(!Lynx::check_argc(pred, "bool", err)){
        return Result(std::move(err));
    }
    auto self = args[0];
    auto val = Lynx::to_bool(self);
    return Result(share(val));
}

// -*-
Result Lynx::fn_integer(const Vec<Self>& args){
    //! @todo: add doc-string of `integer' to lynxDocs describing it syntax

    Error err;
    auto argc = args.size();
    auto pred = (argc==1);
    if(!Lynx::check_argc(pred, "integer", err)){
        return Result(std::move(err));
    }
    auto self = args[0];
    pred = (
        self->is_integer() || self->is_float() ||
        self->is_string() || self->is_bool()
    );
    if(!Lynx::check_type(pred, self, err)){
        err.message() += "\n`integer': expect a boolean, integer, float, "
            "or a numeric string.";
        return Result(std::move(err));
    }
    if(self->is_bool()){
        auto val = *dynamic_cast<Bool*>(self.get());
        return Result(share(val.as_integer()));
    }else if(self->is_integer()){
        auto val = *dynamic_cast<Number*>(self.get());
        return Result(share(val.as_integer()));
    }else if(self->is_float()){
        auto val = *dynamic_cast<Number*>(self.get());
        return Result(share(val.as_integer()));
    }

    auto my_str = *dynamic_cast<String*>(self.get());
    auto numstr = my_str.str();
    size_t pos;
    auto num = static_cast<i64>(std::stoll(numstr, &pos));

    if(pos != numstr.length()){
        std::stringstream ss;
        ss << "" << self->repr() << " is not a numeric string.";
        err = Error(Error::Kind::ValueError, ss.str());
        return Result(std::move(err));
    }
    return Result(share(num));
}

/*
//! @todo: add doc-string of `cond' to lynxDocs describing it syntax

    check_argc(argc, expected_argc, funcname, err)
    
    Error err;
    if(!Lynx::check_type(Symbol("list"), self, err)){
        return Result(std::move(err));
    }
    auto xs = *dynamic_cast<List*>(self.get());
    auto vec = xs.as_vector();

Result Lynx::fn_float(const Vec<Self>& args){}
Result Lynx::fn_complex(const Vec<Self>& args){}
Result Lynx::fn_string(const Vec<Self>& args){}
Result Lynx::fn_list(const Vec<Self>& args){}

// Predicates
Result Lynx::fn_is_nil(const Vec<Self>& args){}
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
Result Lynx::fn_declare_error(const Vec<Self>& args);
Result Lynx::fn_has_feature(const Vec<Self>& args);
Result Lynx::fn_help(const Vec<Self>& args);

*/

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-