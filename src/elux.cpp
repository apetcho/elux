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
#include "elux.hpp"

#include<algorithm>
#include<iostream>
#include<iomanip>
#include<cstdio>
#include<random>
#include<thread> // std::sleep_for
#include<chrono>
#include<stack>

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace ekasoft::klx{
// -
// -*-
Self LiteralExpr::eval(ExprVisitor& v, std::shared_ptr<Env> env) {
    return v.visit(*this, env);
}

// -*-
Self SymbolExpr::eval(ExprVisitor& v, std::shared_ptr<Env> env) {
    return v.visit(*this, env);
}

// -*-
Self ListExpr::eval(ExprVisitor& v, std::shared_ptr<Env> env) {
    return v.visit(*this, env);
}

// =========================
// Environment
// =========================

Env::Env(std::shared_ptr<Env> p)
: parent(std::move(p))
{}

// -*-
bool Env::hasLocal(const std::string& name) const {
    return vars.find(name) != vars.end();
}

// -*-
bool Env::contains(const std::string& name) const{
    if(this->hasLocal(name)){ return true; }
    else{
        if(this->parent==nullptr){
            return false;
        }
        return this->parent->contains(name);
    }
}

// -*-
bool Env::is_immutable(const std::string& name) const{
    if(this->hasLocal(name) && this->immutables.find(name)!=this->immutables.end()){
        return true;
    }else{
        if(this->parent==nullptr){
            return false;
        }
        return this->parent->is_immutable(name);
    }
}

// -*-
void Env::define(const std::string& name, const Self& v) {
    vars[std::string(name)] = v;
}

// -*-
bool Env::set(const std::string& name, const Self& v) {
    auto it = vars.find(name);
    if(it != vars.end()){
        it->second = v;
        return true;
    }
    if(parent){ return parent->set(name, v);}
    return false;
}

// -*-
Self Env::get(const std::string& name) {
    auto it = vars.find(name);
    if(it != vars.end()){ return it->second; }
    if(parent){ return parent->get(name); }
    throw ELuxError(ELuxError::KeyError, "Unbound symbol: " + name);
}

// -*-
bool Env::has_doc(const std::string& name, std::string& docstr) const{
    auto entry = this->docstrings.find(name);
    if(entry==this->docstrings.end()){
        return false;
    }
    docstr = entry->second;
    return true;
}

void Env::add_doc(const std::string& name, const std::string& docstr){
    this->docstrings[name] = docstr;
}


// ===============================
// ELux (Visitor): the interpreter
// ===============================
// -*-
Self ELux::share(void){
    return std::make_shared<Nil>();
}

// -*-
Self ELux::share(i64 val){
    return std::make_shared<Number>(val);
}

// -*-
Self ELux::share(f64 val){
    return std::make_shared<Number>(val);
}

// -*-
Self ELux::share(const Number& val){
    return std::make_shared<Number>(val);
}

// -*-
Self ELux::share(const std::string& val){
    return std::make_shared<String>(val);
}

// -*-
Self ELux::share(const char* val){
    return std::make_shared<String>(val);
}

// -*-
Self ELux::share(char val){
    return std::make_shared<String>(val);
}

// -*-
Self ELux::share(const String& val){
    return std::make_shared<String>(val);
}

// -*-
Self ELux::share(bool val){
    return std::make_shared<Bool>(val);
}

// -*-
Self ELux::share(const Bool& val){
    return std::make_shared<Bool>(val);
}

// -*-
Self ELux::share(const List& val){
    return std::make_shared<List>(val);
}

// -*-
Self ELux::share(const Array& val){
    return std::make_shared<Array>(val);
}

// -*-
Self ELux::share(const HashSet& val){
    return std::make_shared<HashSet>(val);
}

// -*-
Self ELux::share(const HashMap& val){
    return std::make_shared<HashMap>(val);
}

Self ELux::share(const Symbol& val){
    return std::make_shared<Symbol>(val);
}

Self ELux::share(const Pair& val){
    return std::make_shared<Pair>(val);
}

Self ELux::share(const Tuple& val){
    return std::make_shared<Tuple>(val);
}

Self ELux::share(const ELuxError& val){
    return std::make_shared<ELuxError>(val);
}

Expr ELux::share(const LiteralExpr& expr){
    return std::make_shared<LiteralExpr>(expr);
}

Expr ELux::share(const SymbolExpr& expr){
    return std::make_shared<SymbolExpr>(expr);
}

Expr ELux::share(const ListExpr& expr){
    return std::make_shared<ListExpr>(expr);
}

// -*-
bool ELux::is_nil(const Self& self){
    auto ptr = dynamic_cast<Nil*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_bool(const Self& self){
    auto ptr = dynamic_cast<Bool*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_integer(const Self& self){
    auto ptr = dynamic_cast<Number*>(self.get());
    if(ptr==nullptr){ return false; }
    return ptr->is_integer();
}

// -*-
bool ELux::is_float(const Self& self){
    auto ptr = dynamic_cast<Number*>(self.get());
    if(ptr==nullptr){ return false; }
    return !ptr->is_integer();
}

// -*-
bool ELux::is_number(const Self& self){
    auto ptr = dynamic_cast<Number*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_string(const Self& self){
    auto ptr = dynamic_cast<String*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_list(const Self& self){
    auto ptr = dynamic_cast<List*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_array(const Self& self){
    auto ptr = dynamic_cast<Array*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_hashset(const Self& self){
    auto ptr = dynamic_cast<HashSet*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_hashmap(const Self& self){
    auto ptr = dynamic_cast<HashMap*>(self.get());
    if(ptr==nullptr){ return false; }
    return true;
}

// -*-
bool ELux::is_function(const Self& self){
    auto fn = dynamic_cast<Function*>(self.get());
    if(fn==nullptr){ return false; }
    return !fn->isMacro;
}

// -*-
bool ELux::is_macro(const Self& self){
    auto fn = dynamic_cast<Function*>(self.get());
    if(fn==nullptr){ return false; }
    return fn->isMacro;
}

// -*-
bool ELux::is_native(const Self& self){
    auto fn = dynamic_cast<Function*>(self.get());
    if(fn==nullptr){ return false; }
    return !fn->isNative;
}

// -*-
bool ELux::is_callable(const Self& self){
    return (
        ELux::is_function(self) ||
        ELux::is_macro(self) ||
        ELux::is_native(self)
    );
}

// -*-
bool ELux::is_pair(const Self& self){
    auto ptr = dynamic_cast<Pair*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
bool ELux::is_tuple(const Self& self){
    auto ptr = dynamic_cast<Tuple*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
bool ELux::is_iterable(const Self& self){
    auto ptr = dynamic_cast<Iterable*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
bool ELux::is_hashable(const Self& self){
    auto ptr = dynamic_cast<Hashable*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
bool ELux::is_equalable(const Self& self){
    auto ptr = dynamic_cast<Equalable*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
bool ELux::is_comparable(const Self& self){
    auto ptr = dynamic_cast<Comparable*>(self.get());
    return (ptr==nullptr ? false : true);
}

// -*-
std::string ELux::str(const Self& self){
    return self->str();
}

// -*-
Bool ELux::as_bool(const Self& self){
    if(ELux::is_nil(self)){ return Bool(false); }
    if(ELux::is_bool(self)){
        return *dynamic_cast<Bool*>(self.get());
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a Bool");
}

// -*-
f64 ELux::as_float(const Self& self){
    if(ELux::is_number(self)){
        auto num = *dynamic_cast<Number*>(self.get());
        return static_cast<f64>(num);
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a Number");
}

// -*-
i64 ELux::as_integer(const Self& self){
    if(ELux::is_number(self)){
        auto num = *dynamic_cast<Number*>(self.get());
        return static_cast<i64>(num);
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a Number");
}

// -*-
Number ELux::as_number(const Self& self){
    if(ELux::is_number(self)){
        auto num = *dynamic_cast<Number*>(self.get());
        return num;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a Number");
}

// -*-
String ELux::as_string(const Self& self){
    if(ELux::is_string(self)){
        auto str = *dynamic_cast<String*>(self.get());
        return str;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a String");
}

// -*-
Array ELux::as_array(const Self& self){
    if(ELux::is_array(self)){
        auto ans = *dynamic_cast<Array*>(self.get());
        return ans;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected an Array");
}

// -*-
List ELux::as_list(const Self& self){
    if(ELux::is_list(self)){
        auto ans = *dynamic_cast<List*>(self.get());
        return ans;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a List");
}

// -*-
HashSet ELux::as_hashset(const Self& self){
    if(ELux::is_hashset(self)){
        auto ans = *dynamic_cast<HashSet*>(self.get());
        return ans;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a HashSet");
}

// -*-
HashMap ELux::as_hashmap(const Self& self){
    if(ELux::is_hashmap(self)){
        auto ans = *dynamic_cast<HashMap*>(self.get());
        return ans;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a HashMap");
}

// -*-
Function ELux::as_function(const Self& self){
    if(ELux::is_function(self)){
        auto ans = *dynamic_cast<Function*>(self.get());
        return ans;
    }
    throw ELuxError(ELuxError::TypeError, "invalid type. Expected a Function");
}

// -*-
Pair ELux::as_pair(const Self& self){
    Pair pair(ELux::share(), ELux::share());
    if(ELux::is_pair(self)){
        pair = *dynamic_cast<Pair*>(self.get());
    }else if(ELux::is_tuple(self)){
        if(self->len() != 2){
            std::stringstream ss;
            ss << "converting a tuple to a pair. Expect a tuple containing 2 elements but\n";
            ss << "got a tuple containing " << self->len() << " elements.";
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        auto vec = ELux::as_tuple(self).value();
        pair.key = std::move(vec[0]);
        pair.val = std::move(vec[1]);
    }else if(ELux::is_array(self)){
        if(self->len() != 2){
            std::stringstream ss;
            ss << "converting an array to a pair. Expect an array containing 2 elements but\n";
            ss << "got an array containing " << self->len() << " elements.";
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        auto vec = ELux::as_array(self).value();
        pair.key = std::move(vec[0]);
        pair.val = std::move(vec[1]);
    }else if(ELux::is_list(self)){
        if(self->len() != 2){
            std::stringstream ss;
            ss << "converting a list to a pair. Expect a list containing 2 elements but\n";
            ss << "got a list containing " << self->len() << " elements.";
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        auto xs = ELux::as_list(self).value();
        pair.key = std::move(xs.front());
        pair.val = std::move(xs.back());
    }else if(ELux::is_hashset(self)){
        if(self->len() != 2){
            std::stringstream ss;
            ss << "converting a set to a pair. Expect a set containing 2 elements but\n";
            ss << "got a set containing " << self->len() << " elements.";
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        auto xset = ELux::as_hashset(self).value();
        auto vec = Vec<Self>(xset.begin(), xset.end());
        pair.key = std::move(vec[0]);
        pair.val = std::move(vec[1]);
    }else if(ELux::is_hashmap(self)){
        if(self->len() != 1){
            std::stringstream ss;
            ss << "converting a dict to a pair. Expect a dict containing 1 elements but\n";
            ss << "got a dict containing " << self->len() << " elements.";
            throw ELuxError(ELuxError::SyntaxError, ss.str());
        }
        auto xdict = ELux::as_hashmap(self).value();
        Vec<Self> vec{};
        Self key = nullptr;
        Self val = nullptr;
        for(auto [key_, val_]: xdict){
            key = key_;
            val = val_;
        }
        pair.key = std::move(key);
        pair.val = std::move(val);
    }else{
        std::stringstream ss;
        ss << "cannot convert " << std::quoted(self->type().str()) << " to a pair.";
        throw ELuxError(ELuxError::TypeError, ss.str());
    }

    return pair;
}

// -*-
Tuple ELux::as_tuple(const Self& self){
    Vec<Self> vec{};
    if(ELux::is_iterable(self)){
        auto iter = ELux::as_iterator(self);
        vec = {};
        while(!iter->done()){
            vec.push_back(iter->next());
        }
    }else{
        std::stringstream ss;
        ss << "cannot convert " << std::quoted(self->type().str()) << " to a tuple.";
        throw ELuxError(ELuxError::TypeError, ss.str());
    }

    return Tuple(vec);
}

// -*-
Iterator ELux::as_iterator(const Self& self){
    Vec<Self> vec{};
    if(ELux::is_iterable(self)){
        auto iter = ELux::as_iterator(self);
        while(!iter->done()){
            vec.push_back(iter->next());
        }
    }else{
        std::stringstream ss;
        ss << "type " << std::quoted(self->type().str()) << " objects are not iterable.";
        throw ELuxError(ELuxError::TypeError, ss.str());
    }

    return std::make_shared<Array>(vec);
}

// -*-
void ELux::collect(Iterator iter, String& result){
    iter->collect(result);
}

// -*-
void ELux::collect(Iterator iter, Tuple& result){
    iter->collect(result);
}

// -*-
void ELux::collect(Iterator iter, Array& result){
    iter->collect(result);
}

// -*-
void ELux::collect(Iterator iter, List& result){
    iter->collect(result);
}

// -*-
void ELux::collect(Iterator iter, HashSet& result){
    iter->collect(result);
}

// -*-
void ELux::collect(Iterator iter, HashMap& result){
    iter->collect(result);
}

// -*-
bool ELux::is_collection(const Self& self){
    return (
        ELux::is_list(self) ||
        ELux::is_array(self) ||
        ELux::is_hashset(self) ||
        ELux::is_hashmap(self)
    );
}

// -*-
i64 ELux::len(const Self& self){
    if(ELux::is_string(self)){ return ELux::as_string(self).value().length(); }
    if(ELux::is_array(self)){ return ELux::as_array(self).value().size(); }
    if(ELux::is_list(self)){ return ELux::as_list(self).value().size(); }
    if(ELux::is_hashset(self)){ return ELux::as_hashset(self).value().size(); }
    if(ELux::is_hashmap(self)){ return ELux::as_array(self).value().size(); }
    throw ELuxError(ELuxError::SyntaxError, "invalid type. Expected an dict/set/list/array/string");
}

// -*-
void ELux::check_type(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::TypeError, message);
    }
}

// -*-
void ELux::check_value(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::ValueError, message);
    }
}

// -*-
void ELux::check_syntax(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::SyntaxError, message);
    }
}

// -*-
void ELux::check_runtime(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::RuntimeError, message);
    }
}

// -*-
void ELux::check_key(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::KeyError, message);
    }
}

// -*-
void ELux::check_index(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::IndexError, message);
    }
}

// -*-
void ELux::check(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(message);
    }
}

// -*-
void ELux::check_argc(bool pred, const std::string& message){
    if(!pred){
        throw ELuxError(ELuxError::SyntaxError, message);
    }
}

// // -*-
// std::string ELux::repr(const Self& self){
//     //auto expr = valueToExpr(self);
//     struct ToExpr{
//         Expr toExpr(const Self& self){
//             if(ELux::is_list(self)){
//                 auto xs = ELux::as_list(self);
//                 return this->toExpr(xs);
//             }
//             if(ELux::is_string(self)){
//                 return std::make_shared<SymbolExpr>(ELux::as_string(self).value());
//             }
//             return std::make_shared<LiteralExpr>(self);
//         }

//     private:
//         Expr toExpr(const List& xs){
//             auto expr = std::make_shared<ListExpr>();
//             for(auto item: xs.value()){
//                 expr->elements.push_back(this->toExpr(item));
//             }
//             return expr;
//         }
//     };

//     struct ToString{
//         void to_str(const Expr& expr, std::stringstream& ss){
//             if(auto self = dynamic_cast<ListExpr*>(expr.get())){
//                 ss << "(";
//                 auto idx = 0;
//                 for(auto item: self->elements){
//                     if(idx > 0){ ss << " "; }
//                     to_str(item, ss);
//                 }
//                 ss << ")";
//             }else if(auto self=dynamic_cast<SymbolExpr*>(expr.get())){
//                 ss << " " << self->name;
//             }else if(auto self=dynamic_cast<LiteralExpr*>(expr.get())){
//                 ss << self->value->str();
//             }else{
//                 ss << "";
//             }
//         }
//     };

//     std::stringstream ss;
//     auto expr = ToExpr().toExpr(self);
//     ToString().to_str(expr, ss);
//     return ss.str();
// }

// -*-
Self ELux::visit(LiteralExpr& expr, Context env){
    return expr.value;
}

Self ELux::visit(SymbolExpr& expr, Context env){
    return env->get(expr.name.str());
}

Self ELux::eval(Expr expr, Context env) {
    return expr->eval(*this, env);
}


Self ELux::visit(ListExpr& expr, Context env){
    return this->eval(expr.elements, env);
}

// quasiquote helpers
Self ELux::quasiquote(const Self& val, Context env, int depth){
    if(!ELux::is_list(val)){ return val; }
    auto xdata = ELux::as_list(val);
    if(xdata.value().empty()){ return val; }

    // (unquote x)
    if(xdata.value().size()==2 && ELux::is_string(xdata.value().front()) &&
        xdata.value().front()->str()=="unquote" && depth==1){
        auto self = xdata.value().back();
        return this->eval_as_expr(self, env);
    }

    // (unquote-splicing x) only valid inside list
    auto xs = xdata.value();
    Array array{};
    std::for_each(xs.begin(), xs.end(), [this, &array, &depth, &env](const Self& self){
        if(ELux::is_list(self)){
            auto items = ELux::as_list(self).value();
            if(!items.empty() && ELux::str(items.front())=="unquote-splicing" && depth==1){
                if(items.size()!=2){
                    throw ELuxError(ELuxError::SyntaxError, "unquote-splicing i.e ',@' expect exactly 1 argument.");
                }
                auto obj = this->eval_as_expr(items.back(), env);
                if(!ELux::is_list(obj)){
                    throw ELuxError(ELuxError::SyntaxError, "unquote-splicing expects list");
                }
                auto ys = ELux::as_list(obj);
                for(auto y: ys.value()){ array.push(y); }
            }else if(!items.empty() && ELux::str(items.front())=="unquote" && depth==1){
                if(items.size()!=2){
                    throw ELuxError(ELuxError::SyntaxError, "unquote i.e ',' expect exactly 1 argument.");
                }
                auto obj = this->eval_as_expr(items.back(), env);
                array.push(obj);
            }else{
                array.push(ELux::share(ELux::as_list(self)));
            }
        }else{
            array.push(self);
        }
    });

    List result(array);
    return ELux::share(result);
}

// -*-
// evalValueAsExpr
Self ELux::eval_as_expr(const Self& v, Context env) {
    // interpret Value as code (Expr) and evaluate
    if(ELux::is_list(v)){
        const auto& xs = *dynamic_cast<List*>(v.get());
        auto expr = this->to_expr(xs);
        return this->eval(expr, env);
    }else if(ELux::is_string(v)){
        auto expr = std::make_shared<SymbolExpr>(v->str());
        return eval(expr, env);
    } else {
        auto expr = std::make_shared<LiteralExpr>(v);
        return eval(expr, env);
    }
}

// -*-
// valueToExpr
Expr ELux::to_expr(const Self& v) {
    if(ELux::is_list(v)){
        const auto& xs = *dynamic_cast<List*>(v.get());
        return this->to_expr(xs);
    }else if(ELux::is_string(v)){
        return std::make_shared<SymbolExpr>(v->str());
    }else{
        return std::make_shared<LiteralExpr>(v);
    }
}

// valueListToExpr
Expr ELux::to_expr(const List& lst) {
    auto expr = std::make_shared<ListExpr>();
    for(auto& item : lst.value()){
        expr->elements.push_back(std::move(this->to_expr(item)));
    }
    return expr;
}

// =====================
// ELux: list evaluation
// =====================
Self ELux::eval(const Vec<Expr>& elems, Context env) {
    if(elems.empty()){
        return ELux::share();
    }
    // special forms if head is symbol
    if(auto sym = dynamic_cast<SymbolExpr*>(elems[0].get())) {
        const std::string& op = sym->name.str();

        // --------- Special forms ---------
        if(op == "quote"){ return handle_quote(elems, env); }
        if(op == "quasiquote"){ return handle_quasiquote(elems, env); }
        if(op == "unquote" || op == "unquote-splicing"){
            throw ELuxError(ELuxError::SyntaxError,
                "unquote/unquote-splicing only valid inside quasiquote"
            );
        }

        if(op == "if") { return handle_if(elems, env); }
        if(op == "define"){ return handle_define(elems, env); }
        if(op == "var") { return handle_var(elems, env); }
        if(op == "lambda"){ return handle_lambda(elems, env); }
        if(op == "fun"){ return handle_fun(elems, env); }
        if(op == "macro"){ return handle_macro(elems, env); }
        if(op == "let"){ return handle_let(elems, env); }
        if(op == "progn"){ return handle_progn(elems, env); }
        if(op == "while"){ return handle_while(elems, env); }
        if(op == "for"){ return handle_for(elems, env); }
        if(op == "cond"){ return handle_cond(elems, env); }
        if(op == "match"){ return handle_match(elems, env); }
        if(op == "try"){ return handle_try(elems, env); }
        if(op == "throw"){ return handle_throw(elems, env); }
        if(op == "import"){ return handle_import(elems, env); }
        if(op == "export"){ return handle_export(elems, env); }
        // --------- End special forms ---------
    }

    // first element should be operator
    auto headVal = elems[0]->eval(*this, env);
    // function or macro call
    // headVal must be function or macro
    if (!ELux::is_callable(headVal)){
        throw ELuxError(ELuxError::SyntaxError, "first argum must be a callable: " + headVal->str());
    }

    auto fn = dynamic_cast<Function*>(headVal.get());
    fn->elux = this;
    auto argv = this->eval_args(Vec<Expr>(elems.begin()+1, elems.end()), env);
    
    // // macro: receive unevaluated args as Values (AST->Value), expand, then eval
    // if(fn->isMacro || ELux::is_macro(headVal)){
    //     //auto expr = valueToExpr(expand(elems, env));
    //     return fn->call(argv, env);
    // }
    // // normal function
    // // Vec<Self> argv;
    // // for(size_t i = 1; i < elems.size(); ++i){
    // //     argv.push_back(elems[i]->eval(*this, env));
    // // }

    // if(fn->isNative){ return fn->native(argv, env); }

    // if(argv.size() != fn->params.size()){
    //     throw std::runtime_error("function arg count mismatch");
    // }
    // auto callEnv = std::make_shared<Env>(fn->closure);
    // for(size_t i = 0; i < fn->params.size(); ++i){
    //     callEnv->define(fn->params[i], argv[i]);
    // }
    // return fn->body->eval(*this, callEnv);
    return fn->call(argv, env);
}

// // -*-
// Self ELux::expand(const Vec<Expr>& elems, Context env){
//     auto headVal = elems[0]->eval(*this, env);
//     // function or macro call
//     // headVal must be function or macro
//     // if (!ELux::is_callable(headVal)){
//     //     throw std::runtime_error("First element is not callable: " + headVal->str());
//     // }
//     auto fn = *dynamic_cast<Function*>(headVal.get());
//     struct Handler{
//         Self handle(Expr expr){
//             if(auto self = dynamic_cast<LiteralExpr*>(expr.get())){
//                 return self->value;
//             }
//             if(auto self = dynamic_cast<SymbolExpr*>(expr.get())){
//                 return ELux::share(self->name);
//             }
//             if(auto le = dynamic_cast<ListExpr*>(expr.get())){
//                 Array array{};
//                 for(auto& elem : le->elements){
//                     array.push(this->handle(elem));
//                 }

//                 return ELux::share(List(array));
//             }
//             return ELux::share();
//         }
//     };
//     Vec<Self> argv;
//     Handler handler;
//     for(size_t i = 1; i < elems.size(); ++i){
//         argv.push_back(handler.handle(elems[i]));
//     }
//     if(argv.size() != fn.params.size()){
//         throw std::runtime_error("macro arg count mismatch");
//     }
//     auto callEnv = std::make_shared<Env>(fn.closure);
//     for(size_t i = 0; i < fn.params.size(); ++i){
//         callEnv->define(fn.params[i], argv[i]);
//     }
//     // macro body returns Value representing code
//     return fn.body->eval(*this, callEnv);
// }

// -*-
Vec<Self> ELux::eval_args(const Vec<Expr>& elems, Context env){
    struct Handler{
        Self handle(Expr expr){
            if(auto self = dynamic_cast<LiteralExpr*>(expr.get())){
                return self->value;
            }
            if(auto self = dynamic_cast<SymbolExpr*>(expr.get())){
                return ELux::share(self->name);
            }
            if(auto le = dynamic_cast<ListExpr*>(expr.get())){
                Array array{};
                for(auto& elem : le->elements){
                    array.push(this->handle(elem));
                }

                return ELux::share(List(array));
            }
            return ELux::share();
        }
    };
    Vec<Self> argv;
    Handler handler;
    for(size_t i = 1; i < elems.size(); ++i){
        argv.push_back(handler.handle(elems[i]));
    }

    return argv;
}

// -*-
Self ELux::handle_quote(const Vec<Expr>& elems, Context env){
    if (elems.size() != 2){
        throw ELuxError(ELuxError::SyntaxError, "quote expects 1 arg");
    }
    
    struct Handler{
        Self handle(const Expr& expr) const{
            if(auto self=dynamic_cast<LiteralExpr*>(expr.get())){
                return std::move(self->value);
            }
            if(auto self=dynamic_cast<SymbolExpr*>(expr.get())){
                return ELux::share(self->name);
            }
            if(auto self=dynamic_cast<ListExpr*>(expr.get())){
                Array vec{};
                for(auto i=0; i< self->elements.size(); i++){
                    //std::fprintf(stderr, "copy %d\n", i);
                    vec.push(this->handle(self->elements[i]));
                }
                List xs(vec);
                return ELux::share(xs);
            }
            return ELux::share();
        }
    };
    return Handler().handle(std::move(elems[1]));
}

// -*-
Self ELux::handle_quasiquote(const Vec<Expr>& elems, Context env){
    if(elems.size() != 2){
        throw ELuxError(ELuxError::SyntaxError, "quasiquote expects 1 arg");
    }
    
    struct Handler{
        Self handle(Expr expr){
            if(auto self=dynamic_cast<LiteralExpr*>(expr.get())){
                return self->value;
            }
            if(auto self=dynamic_cast<SymbolExpr*>(expr.get())){
                return ELux::share(self->name);
            }
            if(auto self=dynamic_cast<ListExpr*>(expr.get())){
                Array array{};
                for(auto item: self->elements){
                    array.push(this->handle(item));
                }
                return ELux::share(List(array));
            }
            return ELux::share();
        }
    };
    //auto data = conv(elems[1]);
    auto expr = elems[1];
    return quasiquote(Handler().handle(expr), env, 1);
}

// -*-
// Self ELux::handle_unquote(const Vec<Expr>& elems, Context env){
//     throw std::runtime_error("unquote/unquote-splicing only valid inside quasiquote");
// }

/**
 * @brief Define elux's `if' special form.
 * 
 * Syntax
 * ------
 *      (if test yesExpr)
 *      (if test yesEypr noExpr)
 * 
 * @param elems 
 * @param env 
 * @return Self 
 */
Self ELux::handle_if(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size()==3 || exprs.size()==4);
    std::string msg = R"ELUX(
    `if': invalid use of `if'. The correct syntax is one of the following:

    Example
    -------
        (if test expr)
        (if test okExpr noExpr)
    )ELUX";
    auto cond = exprs[1]->eval(*this, env);
    pred = ELux::is_bool(cond);
    ELux::check_argc(pred, msg);
    msg = R"ELUX(
    `if': test condition must evaluate to a bool.

    Example:
    --------
        (if (= 1 1) (println "1 is equal to 1"))
        (if (= 1 0) (println "1 is equal to 0") (println "1 is not equal to 0"))
    )ELUX";
    
    
    if(ELux::as_bool(cond)){
        [[maybe_unused]] auto _ = exprs[2]->eval(*this, env);
    }else if(exprs.size() == 4){
        [[maybe_unused]] auto _ = exprs[3]->eval(*this, env);
    }

    return ELux::share();
}

/**
 * @brief Define elux's `define' special form.
 * 
 * Syntax
 * ------
 *      (define name value)
 *      (define name value doc-string)
 * 
 * @param exprs 
 * @param env 
 * @return Self 
 */
Self ELux::handle_define(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size()==3 || exprs.size()==4);
    std::string msg = R"ELUX(
    `define': invalid use of `define'. The correct syntax is one of the following:

    Example
    -------
        (define name value)
        (define name value "documentation string")
    )ELUX";
    ELux::check_argc(pred, msg);
    auto sym = dynamic_cast<SymbolExpr*>(exprs[1].get());
    msg = R"ELUX(
    `define': first argument must a symbol.

    Example:
    --------
        (define PI 3.14)
        (define E 2.71828 "Euler's number")
    )ELUX";
    ELux::check_syntax(pred, msg);
    
    if(env->immutables.find(sym->name.str())!=env->immutables.end()){
        std::stringstream ss;
        ss << "`define': " << std::quoted(sym->name.str()) << " is immutable.";
        ss << "Cannot redefined an immutable varibale.";
        throw ELuxError(ELuxError::SyntaxError, ss.str());
    }
    auto val = exprs[2]->eval(*this, env);
    env->define(sym->name.str(), val);
    env->immutables.insert(sym->name.str());
    if(exprs.size()==4){
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        pred = ((self!=nullptr) && ELux::is_string(self->value));
        ELux::check_type(
            pred, "`define': expect the third optional argument to be a string if provided"
        );
        env->add_doc(sym->name.str(), self->value->str());
    }
    return ELux::share();
}

/**
 * @brief Define elux's `var' special form.
 * 
 * Syntax
 * ------
 *      (var name value)
 *      (var name value doc-string)
 * 
 * @param exprs 
 * @param env 
 * @return Self 
 */
Self ELux::handle_var(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size()==3 || exprs.size()==4);
    std::string msg = R"ELUX(
    `var': invalid use is one of the following:

    Example
    -------
        (var name value)
        (var name value "documentation string")
    )ELUX";
    ELux::check_argc(pred, msg);
    auto sym = dynamic_cast<SymbolExpr*>(exprs[1].get());
    msg = R"ELUX(
    `var': first argument must a symbol.

    Example:
    --------
        (var PI 3.14)
        (var E 2.71828 "Euler's number")
    )ELUX";
    ELux::check_syntax(pred, msg);
    
    if(env->immutables.find(sym->name.str())!=env->immutables.end()){
        std::stringstream ss;
        ss << "`var': " << std::quoted(sym->name.str()) << " is immutable.";
        ss << "Cannot redefined an immutable varibale.";
        throw ELuxError(ELuxError::SyntaxError, ss.str());
    }
    auto val = exprs[2]->eval(*this, env);
    if(env->contains(sym->name.str())){
        env->set(sym->name.str(), val);
    }else{
        env->define(sym->name.str(), val);
    }
    if(exprs.size()==4){
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        pred = ((self!=nullptr) && ELux::is_string(self->value));
        ELux::check_type(
            pred, "`var': expect the third optional argument to be a string if provided"
        );
        env->add_doc(sym->name.str(), self->value->str());
    }
    return ELux::share();
}

/**
 * @brief Define elux's `lambda' special form.
 * 
 * Syntax
 * ------
 *      (lambda params body)
 * 
 * @param elems 
 * @param env 
 * @return Self 
 */
Self ELux::handle_lambda(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size() < 3);
    auto msg = R"ELUX(
    `lambda': malformed lambda expression. The correct way is as follows:

    Syntax:
    -------
        (lambda params body)

    Example
    -------
        (lambda (name) (println "Hello " name "!"))
    )ELUX";
    ELux::check_argc(pred, msg);
    
    auto params_ = dynamic_cast<ListExpr*>(exprs[1].get());
    pred = (params_!=nullptr);
    ELux::check_type(pred, "`lambda': params must be a list.");
    Vec<std::string> params;
    for(auto& param : params_->elements) {
        auto sym = dynamic_cast<SymbolExpr*>(param.get());
        if (!sym){
            std::stringstream ss;
            ss << "`lambda': invalid parameter in the parameters list.\n";
            ss << "Parameters must by symbols literal but got ";
            ss << std::quoted(sym->name.type().str()) << " object.";
            throw ELuxError(ELuxError::TypeError, ss.str());
        }
        params.push_back(sym->name.str());
    }
    // body: if multiple forms, wrap in (progn ...)
    Expr body;
    if(exprs.size() == 3){
        body = exprs[2];
    }else{
        auto le = std::make_shared<ListExpr>();
        le->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        for(size_t i = 2; i < exprs.size(); ++i){
            le->elements.push_back(exprs[i]);
        }
        body = le;
    }
    auto lambda = std::make_shared<Function>();
    lambda->params = params;
    lambda->body = body;
    lambda->closure = env;
    lambda->isMacro = false;
    lambda->elux = this;
    lambda->name = std::nullopt;
    return std::move(lambda);
}

/**
 * @brief Define elux's `fun' special form.
 * 
 * Syntax
 * ------
 *      (fun name params body)
 *      (fun name params doc-string body)
 * 
 * Example
 * -------
 *      (fun hello (name)
 *          (println "Hello " name "!"))
 * 
 *      (fun say-bye (name)
 *          "Say goodbye to `NAME'."
 *          (println "Goodbye " name "!"))
 * 
 * @param elems 
 * @param env 
 * @return Self 
 */
Self ELux::handle_fun(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size() < 4);
    auto msg = R"ELUX(
    `fun': malformed `fun` expression. The correct syntax is as follows:

    Syntax
    ------
        (fun name params body)
        (fun name params doc-string body)

    Examples
    --------
        (fun hello (name)
            (println "Hello " name "!"))

        (fun say-bye (name)
            "Say goodbye to `NAME'."
            (println "Goodbye " name "!"))
    )ELUX";
    ELux::check_syntax(pred, msg);
    
    auto funcsym = dynamic_cast<SymbolExpr*>(exprs[1].get());
    pred = (funcsym!=nullptr);
    ELux::check_type(pred, "`fun': name must be a symbol");
    
    auto params_ = dynamic_cast<ListExpr*>(exprs[2].get());
    pred = (params_!=nullptr);
    ELux::check_type(pred, "`fun': params must be a list.");
    
    std::vector<std::string> params;
    for(auto& param : params_->elements) {
        auto sym = dynamic_cast<SymbolExpr*>(param.get());
        pred = (params_!=nullptr);
        ELux::check_type(pred, "`fun': parameters in parameters list must be symbols.");
        params.push_back(sym->name.str());
    }
    Expr body;
    std::string help{};
    if(exprs.size() == 4) {
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        pred = (
            (self != nullptr) && ELux::is_string(self->value)
        );
        if(pred){ // we have a doc-string and an empty body
            body = std::make_shared<ListExpr>(
                Vec<Expr>{std::make_shared<SymbolExpr>("progn")} // (progn)
            );
            help = self->value->repr();
        }else{ // no doc-string but some
            body = exprs[3];
        }
    }else{
        auto expr = std::make_shared<ListExpr>();
        // Create: (progn expr1 expr2 ... exprN)
        expr->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        if(self!=nullptr && ELux::is_string(self->value)){ // we have a doc-string
            help = self->value->repr();
            if(exprs.size()==4){ // empty body
                body = std::make_shared<ListExpr>(
                    Vec<Expr>{std::make_shared<SymbolExpr>("progn")}
                );
            }else{ // non-exmpty body
                for(size_t i = 4; i < exprs.size(); ++i){
                    expr->elements.push_back(exprs[i]);
                }
                body = expr;
            }
        }else{ // no doc-string
            for(size_t i = 3; i < exprs.size(); ++i){
                expr->elements.push_back(exprs[i]);
            }
            body = expr;
        }
    }
    auto func = std::make_shared<Function>();
    func->params = params;
    func->body = body;
    func->closure = env;
    func->isMacro = false;
    func->elux = this;
    func->name = funcsym->name.str();
    env->define(funcsym->name.str(), func);
    env->add_doc(funcsym->name.str(), help);
    return func;
}

// -*-
Self ELux::handle_macro(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size() < 4);
    auto msg = R"ELUX(
    `macro': malformed `macro' expression. The correct syntax is as follows:

    Syntax
    ------
        (macro name params body)
        (macro name params doc-string body)

    Examples
    --------
        (macro hello (name)
            `(println "Hello " ,name "!"))

        (fun say-bye (name)
            "Say goodbye to `NAME'."
            `(println "Goodbye " ,name "!"))
    )ELUX";
    ELux::check_syntax(pred, msg);
    
    auto funcsym = dynamic_cast<SymbolExpr*>(exprs[1].get());
    pred = (funcsym!=nullptr);
    ELux::check_type(pred, "`macro': name must be a symbol");
    
    auto params_ = dynamic_cast<ListExpr*>(exprs[2].get());
    pred = (params_!=nullptr);
    ELux::check_type(pred, "`fun': params must be a list.");
    
    std::vector<std::string> params;
    for(auto& param : params_->elements) {
        auto sym = dynamic_cast<SymbolExpr*>(param.get());
        pred = (params_!=nullptr);
        ELux::check_type(pred, "`fun': parameters in parameters list must be symbols.");
        params.push_back(sym->name.str());
    }
    
    Expr body;
    std::string help{};
    if(exprs.size() == 4) {
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        pred = (
            (self != nullptr) && ELux::is_string(self->value)
        );
        if(pred){ // we have a doc-string and an empty body
            body = std::make_shared<ListExpr>(
                Vec<Expr>{std::make_shared<SymbolExpr>("progn")} // (progn)
            );
            help = self->value->repr();
        }else{ // no doc-string but some
            body = exprs[3];
        }
    }else{
        auto expr = std::make_shared<ListExpr>();
        // Create: (progn expr1 expr2 ... exprN)
        expr->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        auto self = dynamic_cast<LiteralExpr*>(exprs[3].get());
        if(self!=nullptr && ELux::is_string(self->value)){ // we have a doc-string
            help = self->value->repr();
            if(exprs.size()==4){ // empty body
                body = std::make_shared<ListExpr>(
                    Vec<Expr>{std::make_shared<SymbolExpr>("progn")}
                );
            }else{ // non-exmpty body
                for(size_t i = 4; i < exprs.size(); ++i){
                    expr->elements.push_back(exprs[i]);
                }
                body = expr;
            }
        }else{ // no doc-string
            for(size_t i = 3; i < exprs.size(); ++i){
                expr->elements.push_back(exprs[i]);
            }
            body = expr;
        }
    }
    // -*-
    
    
    auto macro = std::make_shared<Function>();
    macro->params = params;
    macro->body = body;
    macro->closure = env;
    macro->isMacro = true;
    macro->elux = this;
    macro->name = funcsym->name.str();
    env->define(funcsym->name.str(), macro);
    env->add_doc(funcsym->name.str(), help);
    return macro;
}

// -*-
Self ELux::handle_let(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size() < 3);
    auto msg = R"ELUX(
    `let': malformed `let' expression. The correct syntax is as follows

    Syntax
    ------
        (let bindings body)

    Example
    -------
        (let ((x 2) (y 5))
            (println x " + " y " = " (+ x y) ))
    )ELUX";
    ELux::check_argc(pred, msg);
    
    auto bindings = dynamic_cast<ListExpr*>(exprs[1].get());
    pred = (bindings != nullptr);
    ELux::check_type(pred, "`let': bindings must be a list.");
    
    auto localEnv = std::make_shared<Env>(env);
    for(auto& binding : bindings->elements) {
        auto keyval = dynamic_cast<ListExpr*>(binding.get());
        pred = (keyval != nullptr || keyval->elements.size() != 2);
        ELux::check_syntax(pred, "`let': bindings element must be (name value) pairs.");
        auto sym = dynamic_cast<SymbolExpr*>(keyval->elements[0].get());
        pred = (sym!=nullptr);
        ELux::check_type(
            pred, "`let': first element of each binding must be a symbol"
        );
        auto val = keyval->elements[1]->eval(*this, env);
        localEnv->define(sym->name.str(), val);
    }
    // body
    for(size_t i = 2; i < exprs.size(); ++i){
        [[maybe_unused]] auto _ = exprs[i]->eval(*this, localEnv);
    }
    return ELux::share();
}

// -*-
Self ELux::handle_progn(const Vec<Expr>& exprs, Context env){
    for(size_t i = 1; i < exprs.size(); ++i) {
        [[maybe_unused]] auto _ = exprs[i]->eval(*this, env);
    }
    return ELux::share();
}

// -*-
Self ELux::handle_while(const Vec<Expr>& exprs, Context env){
    auto pred = (exprs.size() < 3);
    auto msg = R"ELUX(
    `while': malformed `while' form. The correct syntax is as follows:

    Syntax
    ------
        (while testExpr body)

    Example
    -------
        (var x 0)
        (while (< x 5)
            (println "x = " x))
    )ELUX";
    ELux::check_argc(pred, msg);
    auto localEnv = std::make_shared<Env>(env);
    
    while(true){
        auto cond = exprs[1]->eval(*this, env);
        pred = ELux::is_bool(cond);
        std::stringstream ss;
        ss << "`while': expect `testExpr' to evaluate to a bool, but got ";
        ss << std::quoted(cond->type().str()) << " object.";
        ELux::check_type(pred, ss.str());
        if(!ELux::as_bool(cond)){ break;}
        for(size_t i = 2; i < exprs.size(); ++i){
            [[maybe_unused]] auto _ = exprs[i]->eval(*this, localEnv);
        }
    }
    return ELux::share();
}

// -*-
Self ELux::handle_for(const Vec<Expr>& exprs, Context env){
    // (for (var list-expr) body...)
    auto pred = (exprs.size() < 3);
    auto msg = R"ELUX(
    `for': malformed `for' expression. The correct  syntax is as follows:

    Syntax
    ------
        (for (var iterable) body)

    Example
    -------
        (var xs (List 1 2 3 4 5 6))
        (for (x xs)
            (println "x = " x))
    )ELUX";
    ELux::check_argc(pred, msg);
    
    auto binding = dynamic_cast<ListExpr*>(exprs[1].get());
    ELux::check_type(pred, "`for': the first argument must be a (var iterable) pair.");
    pred = (binding != nullptr || binding->elements.size() != 2);
    ELux::check_type(pred, "`for': the second element of the first argument must be an iterable.");
    
    auto varsym = dynamic_cast<SymbolExpr*>(binding->elements[0].get());
    pred = (varsym != nullptr);
    ELux::check_type(pred, "`for': the first element of the first argument must be an symbol.");
    
    auto data = binding->elements[1]->eval(*this, env);
    auto iterator = dynamic_cast<Iterable*>(data.get());
    auto localEnv = std::make_shared<Env>(env);
    auto var = varsym->name.str();
    localEnv->define(var, ELux::share());
    while(!iterator->done()){
        auto self = iterator->next();
        localEnv->set(var, self);
        // Evaluate the body of the for-loop
        for(size_t i=2; i < exprs.size(); i++){
            [[maybe_unused]] auto _ = exprs[i]->eval(*this, localEnv);
        }
    }
    return ELux::share();
}

// -*-
Self ELux::handle_cond(const Vec<Expr>& elems, Context env){
    // (cond (test expr...) (test expr...) (t expr...))
    for(size_t i = 1; i < elems.size(); ++i){
        auto clause = dynamic_cast<ListExpr*>(elems[i].get());
        if(!clause || clause->elements.empty()){
            throw ELuxError(ELuxError::SyntaxError, "cond clause must be list");
        }
        auto testExpr = clause->elements[0];
        bool isElse = false;
        if(auto s = dynamic_cast<SymbolExpr*>(testExpr.get())){
            if(s->name.str() == "true"){ isElse = true; }
        }
        if(isElse || ELux::as_bool(testExpr->eval(*this, env))){
            Self result = ELux::share();
            for(size_t j = 1; j < clause->elements.size(); ++j){
                result = clause->elements[j]->eval(*this, env);
            }
            return result;
        }
    }
    return ELux::share();
}

// -*-
Self ELux::handle_match(const Vec<Expr>& elems, Context env){
    // very simple: (match value (pattern expr) (pattern expr) ...)
    // patterns only support literals and t as wildcard
    if(elems.size() < 3){
        throw ELuxError(ELuxError::SyntaxError, "match expects value and clauses");
    }
    auto mval = elems[1]->eval(*this, env);
    for(size_t i = 2; i < elems.size(); ++i){
        auto clause = dynamic_cast<ListExpr*>(elems[i].get());
        if(!clause || clause->elements.size() < 2){
            throw ELuxError(ELuxError::SyntaxError, "match clause must be (pattern expr)");
        }
        auto patExpr = clause->elements[0];
        bool wildcard = false;
        if(auto s = dynamic_cast<SymbolExpr*>(patExpr.get())){
            if(s->name.str() == "true"){ wildcard = true; }
        }
        bool matched = false;
        if(wildcard){ matched = true; }
        else{
            auto patVal = patExpr->eval(*this, env);
            matched = ((patVal->str() == mval->str()));
        }
        if(matched){
            Self result = ELux::share();
            for(size_t j = 1; j < clause->elements.size(); ++j){
                result = clause->elements[j]->eval(*this, env);
            }
            return result;
        }
    }
    return ELux::share();
}

// -*-
Self ELux::handle_try(const Vec<Expr>& elems, Context env){
    // (try body... (catch var body...))
    // find catch clause
    size_t catchIndex = 0;
    for(size_t i = 1; i < elems.size(); ++i){
        if(auto se = dynamic_cast<SymbolExpr*>(elems[i].get())){
            if(se->name.str() == "catch") {
                catchIndex = i;
                break;
            }
        }
    }
    if(!catchIndex){
        throw ELuxError(ELuxError::SyntaxError, "try must contain catch");
    }
    Self result = ELux::share();
    try{
        for(size_t i = 1; i < catchIndex; ++i){
            result = elems[i]->eval(*this, env);
        }
        //return result;
    }catch(const std::exception& ex){
        if(catchIndex + 2 > elems.size()){
            throw ELuxError(ELuxError::SyntaxError, "catch must be (catch var body...)");
        }
        auto varSym = dynamic_cast<SymbolExpr*>(elems[catchIndex+1].get());
        if(!varSym){
            throw ELuxError(ELuxError::SyntaxError, "catch var must be symbol");
        }
        auto newEnv = std::make_shared<Env>(env);
        newEnv->define(varSym->name.str(), std::make_shared<String>(ex.what()));
        for(size_t i = catchIndex+2; i < elems.size(); ++i){
            result = elems[i]->eval(*this, newEnv);
        }
        //return result;
    }
    return result;
}

// -*-
Self ELux::handle_throw(const Vec<Expr>& elems, Context env){
    //! @todo: implement this
    throw ELuxError(ELuxError::RuntimeError, "`throw' is not implemented yet.");
}

// -*-
//! @todo: refactor this functions
Self ELux::handle_import(const Vec<Expr>& elems, Context env){
    // (import "file.elux")
    if(elems.size() != 2){
        throw ELuxError(ELuxError::SyntaxError, "import expects filename");
    }
    auto lit = dynamic_cast<LiteralExpr*>(elems[1].get());
    if(!lit || !ELux::is_string(lit->value)){
        throw ELuxError(ELuxError::SyntaxError, "import expects string filename");
    }
    std::string filename = dynamic_cast<String*>(lit->value.get())->str();
    std::ifstream in(filename);
    if(!in){
        throw ELuxError(ELuxError::RuntimeError, "Cannot open module file: " + filename);
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    Parser parser(buffer.str());
    in.close();
    auto exprs = parser.parse();
    auto moduleEnv = std::make_shared<Env>(env);
    ELux elux;
    Self last = ELux::share();
    for(auto& expr : exprs){
        last = elux.eval(expr, moduleEnv);
    }
    // collect exports: module must define (exports 'a 'b ...)
    Self exportsVal;
    try{
        exportsVal = moduleEnv->get("exports");
    }catch(...){
        // no exports, return nil
        return ELux::share();
    }
    if( !ELux::is_list(exportsVal)){
        throw ELuxError(ELuxError::RuntimeError, "exports must be list of symbols (strings)");
    }
    HashMap dict;
    const List& exList = *dynamic_cast<List*>(exportsVal.get());
    for(auto& symVal : exList.value()){
        if(!ELux::is_string(symVal)){
            throw ELuxError(ELuxError::RuntimeError, "exports entries must be strings");
        }
        std::string name = dynamic_cast<String*>(symVal.get())->str();
        dict.m_hmap[ELux::share(name)] = moduleEnv->get(name);
    }
    return std::make_shared<HashMap>(dict);
}

// -*-
Self ELux::handle_export(const Vec<Expr>& elems, Context env){
    //! @todo: implement this
    throw ELuxError(ELuxError::RuntimeError, "`export' is not implemented yet.");
}

// -*-
//! @todo: refactor & reimplement this method
void ELux::run(const std::string& code, Context env, const std::string& label) {
    auto len = 4 + label.length() + 4;
    std::string line(len, '=');
    std::cout << line << std::endl;
    std::cout << "=*= " << label << " =*=" << std::endl;
    std::cout << line << std::endl;

    try{
        Parser p(code);
        auto exprs = p.parse();
        ELux elux;
        Self last = ELux::share();
        for(auto& expr : exprs) {
            [[maybe_unused]] auto _ = elux.eval(expr, env);
        }
        //std::cout << "Result: " << toString(last) << "\n\n";
    }catch(const ELuxError& err){
        std::cerr << "\x1b[31m" << err.kind().str() << "\x1b[0m: " << err.str() << std::endl;
    }catch(const std::exception& err){
        std::cerr << "\x1b[31mError\x1b[0m: " << err.what() << std::endl;
    }
}

//! @todo
/*
void ELux::repl(const Vec<std::string>& args){}
void ELux::setup(void){}
*/

// -*-
size_t ModuleHash::operator()(const ModulePtr& self) const{
    auto key = self->key();
    return std::hash<std::string>{}(key);
}

// -*-
bool ModuleEqual::operator()(const ModulePtr& lhs, const ModulePtr& rhs) const{
    return (lhs->key()==rhs->key());
}

// -*-
Module::Module(ELux* elux, const Symbol& name)
: m_elux{elux}
, m_name{name}{
    this->setup(name);
}

// -*-
Module::Module(ELux* elux, const fs::path& modulePath)
: m_elux{elux}
, m_name{Symbol("")}
{
    this->setup(modulePath);
}
// -*-
Module::Module(ELux* elux, const Symbol& sym, const fs::path& modulePath)
: m_elux{elux}
, m_name{Symbol("")}
{
    this->setup(sym, modulePath);
}

// -*-
Module::Module(Module&& other) noexcept
: m_elux{std::move(other.m_elux)}
, m_name{std::move(other.m_name)}
, m_path{std::move(other.m_path)}
, m_filename{std::move(other.m_filename)}
, m_env{std::move(other.m_env)}
{
    other.m_elux = nullptr;
    other.m_env = nullptr;
    other.m_filename = "";
    other.m_path = "";
    other.m_name = Symbol("");
}

// -*-
Module& Module::operator=(Module&& other) noexcept{
    if(this != &other){
        this->m_elux = std::move(other.m_elux);
        this->m_name = std::move(other.m_name);
        this->m_path = std::move(other.m_path);
        this->m_filename = std::move(other.m_filename);
        this->m_env = std::move(other.m_env);
        other.m_elux = nullptr;
        other.m_env = nullptr;
        other.m_filename = "";
        other.m_path = "";
        other.m_name = Symbol("");
    }
    return *this;
}

// -*-
const std::string& Module::key(void) const{
    std::stringstream ss;
    ss << this->m_path.string() << "::";
    ss << this->m_filename << "::";
    ss << this->m_name.str();
    return ss.str();
}

// -*-
std::string Module::name_from_key(const std::string& modKey){
    std::string token{modKey};
    if(!Module::is_module_key(token)){
        std::stringstream ss;
        ss << token << " is not a module key.";
        throw ELuxError(ELuxError::RuntimeError, ss.str());
    }
    auto pos = token.find("::");
    token = token.substr(pos+2);
    pos = token.find("::");
    token = token.substr(pos+2);

    return token;
}

bool Module::is_module_key(const std::string& token){
    // Expect key = path::filename::modulename
    auto text = std::string{token};
    auto pos = text.find("::");
    if(pos==std::string::npos){ return false; }
    text = text.substr(pos+2);
    pos = text.find("::");
    if(pos==std::string::npos){ return false; }
    text = text.substr(pos+2);
    pos = text.find("::");
    if(pos!=std::string::npos){ return false; }
    return true;
}

// -*-
bool Module::is_builtin_module(void) const{
    auto myKey = this->key();
    auto path = myKey.substr(0, myKey.find("::"));
    return (this->m_path=="@elux");
}

// -*-
void Module::setup(const Symbol& sym){
    this->m_name = sym;
    this->m_path = "@elux";
    this->m_filename = ("__elux__" + sym.str());
    bool found{false};
    for(auto& mymod: ELux::myModules){
        if(mymod->key()==this->key()){
            this->m_env = mymod->m_env;
            found = true;
            break;
        }
    }

    if(!found){
        std::stringstream ss;
        ss << "Module " << std::quoted(sym.str()) << "not found";
        throw ELuxError(ELuxError::RuntimeError, ss.str());
    }
}

// -*-
void Module::setup(const fs::path& path){
    this->m_path = path;
    this->m_filename = this->m_path.filename();
    auto pos = this->m_filename.find(ELux::myExt);
    if(pos==std::string::npos){
        std::stringstream ss;
        ss << "ELux script filename must always have " << std::quoted(ELux::myExt);
        ss << " extension.";
        throw ELuxError(ELuxError::RuntimeError, ss.str());
    }
    if(!fs::exists(path)){
        std::stringstream ss;
        ss << "Module " << std::quoted(path.string()) << "not found";
        throw ELuxError(ELuxError::RuntimeError, ss.str());
    }

    auto size = fs::file_size(path);
    std::ifstream fin(path, std::ios::binary);
    std::string src(size, '\0');
    fin.read(&src[0], size);
    fin.close();

    auto program = Parser(src).parse();
    this->m_env = std::make_shared<Env>(this->m_elux->runtime());
    for(auto expr: program){
        [[maybe_unused]] auto _ = this->m_elux->eval({expr}, this->m_env);
    }
}

// -*-
void Module::setup(const Symbol& sym, const fs::path& path){
    this->setup(path);
    this->m_name = sym;
}


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-