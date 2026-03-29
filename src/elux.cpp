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

// -*-
Self ELux::handle_if(const Vec<Expr>& elems, Context env){
    if (elems.size() < 3 || elems.size() > 4){
        throw ELuxError(ELuxError::SyntaxError, "if expects 2 or 3 args");
    }
    auto cond = elems[1]->eval(*this, env);
    if(ELux::as_bool(cond)){
        return elems[2]->eval(*this, env);
    }else if(elems.size() == 4){
        return elems[3]->eval(*this, env);
    }else{
        return ELux::share();
    }
}

// -*-
Self ELux::handle_define(const Vec<Expr>& elems, Context env){
    if(elems.size() != 3){
        throw ELuxError(ELuxError::SyntaxError, "define expects name and value");
    }
    auto symExpr = dynamic_cast<SymbolExpr*>(elems[1].get());
    if(!symExpr){
        throw ELuxError(ELuxError::SyntaxError, "define name must be symbol");
    }
    if(env->immutables.find(symExpr->name.str())!=env->immutables.end()){
        std::stringstream ss;
        ss << std::quoted(symExpr->name.str()) << " is immutable.";
        ss << "Cannot redefined an immutable varibale.";
        throw ELuxError(ELuxError::SyntaxError, ss.str());
    }
    auto val = elems[2]->eval(*this, env);
    env->define(symExpr->name.str(), val);
    env->immutables.insert(symExpr->name.str());
    return val;
}

// -*-
Self ELux::handle_var(const Vec<Expr>& elems, Context env){
    if(elems.size() != 3){
        throw ELuxError(ELuxError::SyntaxError, "var expects name and value");
    }
    auto symExpr = dynamic_cast<SymbolExpr*>(elems[1].get());
    if(!symExpr){
        throw ELuxError(ELuxError::SyntaxError, "var name must be symbol");
    }
    if(env->immutables.find(symExpr->name.str())!=env->immutables.end()){
        std::stringstream ss;
        ss << std::quoted(symExpr->name.str()) << " is immutable. ";
        ss << "Cannot update an immutable varibale.";
        throw ELuxError(ELuxError::SyntaxError, ss.str());
    }
    auto val = elems[2]->eval(*this, env);
    Self old;
    if(env->contains(symExpr->name.str())){
        old = env->get(symExpr->name.str());
        env->set(symExpr->name.str(), val);
    }else{
        old = val;
        env->define(symExpr->name.str(), val);
    }
    return old;
}

// -*-
Self ELux::handle_lambda(const Vec<Expr>& elems, Context env){
    if(elems.size() < 3){
        throw ELuxError(ELuxError::SyntaxError, "lambda expects params and body");
    }
    auto paramsList = dynamic_cast<ListExpr*>(elems[1].get());
    if(!paramsList){
        throw ELuxError(ELuxError::SyntaxError, "lambda params must be list");
    }
    std::vector<std::string> params;
    for(auto& p : paramsList->elements) {
        auto s = dynamic_cast<SymbolExpr*>(p.get());
        if (!s){ throw ELuxError(ELuxError::SyntaxError, "lambda param must be symbol"); }
        params.push_back(s->name.str());
    }
    // body: if multiple forms, wrap in (progn ...)
    Expr body;
    if(elems.size() == 3){
        body = elems[2];
    }else{
        auto le = std::make_shared<ListExpr>();
        le->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        for(size_t i = 2; i < elems.size(); ++i){
            le->elements.push_back(elems[i]);
        }
        body = le;
    }
    auto fn = std::make_shared<Function>();
    fn->params = params;
    fn->body = body;
    fn->closure = env;
    fn->isMacro = false;
    fn->elux = this;
    fn->name = std::nullopt;
    return std::move(fn);
}

// -*-
Self ELux::handle_fun(const Vec<Expr>& elems, Context env){
    if(elems.size() < 4){
        throw ELuxError(ELuxError::SyntaxError, "fun expects name, params, body");
    }
    auto nameSym = dynamic_cast<SymbolExpr*>(elems[1].get());
    if(!nameSym){
        throw ELuxError(ELuxError::SyntaxError, "fun name must be symbol");
    }
    auto paramsList = dynamic_cast<ListExpr*>(elems[2].get());
    if(!paramsList){
        throw ELuxError(ELuxError::SyntaxError, "fun params must be list");
    }
    std::vector<std::string> params;
    for(auto& p : paramsList->elements) {
        auto s = dynamic_cast<SymbolExpr*>(p.get());
        if(!s){
            throw ELuxError(ELuxError::SyntaxError, "fun param must be symbol");
        }
        params.push_back(s->name.str());
    }
    Expr body;
    if(elems.size() == 4) {
        body = elems[3];
    }else{
        auto le = std::make_shared<ListExpr>();
        le->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        for(size_t i = 3; i < elems.size(); ++i){
            le->elements.push_back(elems[i]);
        }
        body = le;
    }
    auto fn = std::make_shared<Function>();
    fn->params = params;
    fn->body = body;
    fn->closure = env;
    fn->isMacro = false;
    fn->elux = this;
    fn->name = nameSym->name.str();
    env->define(nameSym->name.str(), fn);
    return fn;
}

// -*-
Self ELux::handle_macro(const Vec<Expr>& elems, Context env){
    if(elems.size() < 4){
        throw ELuxError(ELuxError::SyntaxError, "macro expects name, params, body");
    }
    auto nameSym = dynamic_cast<SymbolExpr*>(elems[1].get());
    if(!nameSym){
        throw ELuxError(ELuxError::SyntaxError, "macro name must be symbol");
    }
    auto paramsList = dynamic_cast<ListExpr*>(elems[2].get());
    if(!paramsList){
        throw ELuxError(ELuxError::SyntaxError, "macro params must be list");
    }
    std::vector<std::string> params;
    for(auto& p : paramsList->elements){
        auto s = dynamic_cast<SymbolExpr*>(p.get());
        if(!s){
            throw ELuxError(ELuxError::SyntaxError, "macro param must be symbol");
        }
        params.push_back(s->name.str());
    }
    Expr body;
    if(elems.size() == 4) {
        body = elems[3];
    }else{
        auto le = std::make_shared<ListExpr>();
        le->elements.push_back(std::make_shared<SymbolExpr>("progn"));
        for(size_t i = 3; i < elems.size(); ++i){
            le->elements.push_back(elems[i]);
        }
        body = le;
    }
    auto fn = std::make_shared<Function>();
    fn->params = params;
    fn->body = body;
    fn->closure = env;
    fn->isMacro = true;
    fn->elux = this;
    fn->name = nameSym->name.str();
    env->define(nameSym->name.str(), fn);
    return fn;
}

// -*-
Self ELux::handle_let(const Vec<Expr>& elems, Context env){
    if(elems.size() < 3){
        throw ELuxError(ELuxError::SyntaxError, "let expects bindings and body");
    }
    auto bindingsList = dynamic_cast<ListExpr*>(elems[1].get());
    if(!bindingsList){
        throw ELuxError(ELuxError::SyntaxError, "let bindings must be list");
    }
    auto newEnv = std::make_shared<Env>(env);
    for(auto& b : bindingsList->elements) {
        auto pairList = dynamic_cast<ListExpr*>(b.get());
        if(!pairList || pairList->elements.size() != 2){
            throw ELuxError(ELuxError::SyntaxError, "let binding must be (name value)");
        }
        auto nameSym = dynamic_cast<SymbolExpr*>(pairList->elements[0].get());
        if(!nameSym){
            throw ELuxError(ELuxError::SyntaxError, "let binding name must be symbol");
        }
        auto val = pairList->elements[1]->eval(*this, env);
        newEnv->define(nameSym->name.str(), val);
    }
    // body
    Self result;
    for(size_t i = 2; i < elems.size(); ++i) {
        result = elems[i]->eval(*this, newEnv);
    }
    return result;
}

// -*-
Self ELux::handle_progn(const Vec<Expr>& elems, Context env){
    Self result = ELux::share();
    for(size_t i = 1; i < elems.size(); ++i) {
        result = elems[i]->eval(*this, env);
    }
    return result;
}

// -*-
Self ELux::handle_while(const Vec<Expr>& elems, Context env){
    if(elems.size() < 3){
        throw ELuxError(ELuxError::SyntaxError, "while expects condition and body");
    }
    auto newEnv = std::make_shared<Env>(env);
    //Value result;
    while(true){
        auto cond = elems[1]->eval(*this, env);
        if(!ELux::as_bool(cond)){ break;}
        for(size_t i = 2; i < elems.size(); ++i){
            //result = elems[i]->accept(*this, env);
            [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
        }
    }
    return ELux::share();
}

// -*-
Self ELux::handle_for(const Vec<Expr>& elems, Context env){
    // (for (var list-expr) body...)
    if (elems.size() < 3){
        throw ELuxError(ELuxError::SyntaxError, "for expects (var list) and body");
    }
    auto binding = dynamic_cast<ListExpr*>(elems[1].get());
    if(!binding || binding->elements.size() != 2){
        throw ELuxError(ELuxError::SyntaxError, "for binding must be (var list-expr)");
    }
    auto varSym = dynamic_cast<SymbolExpr*>(binding->elements[0].get());
    if(!varSym){
        throw ELuxError(ELuxError::SyntaxError, "for var must be symbol");
    }
    auto listVal = binding->elements[1]->eval(*this, env);
    auto failed = (
        !ELux::is_list(listVal) &&
        !ELux::is_array(listVal) &&
        !ELux::is_hashset(listVal) &&
        !ELux::is_hashmap(listVal) &&
        !ELux::is_string(listVal)
    );
    if(failed){
        throw ELuxError(ELuxError::SyntaxError, "for expects list/array/set/dict/string");
    }
    auto newEnv = std::make_shared<Env>(env);
    // Value result;
    if(ELux::is_list(listVal)){
        const auto& xs = *dynamic_cast<List*>(listVal.get());
        for(auto& item : xs.value()){
            newEnv->define(varSym->name.str(), item);
            for (size_t i = 2; i < elems.size(); ++i) {
                // result = elems[i]->accept(*this, newEnv);
                [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
            }
        }
    }else if(ELux::is_array(listVal)){
        const Array& array = *dynamic_cast<Array*>(listVal.get());
        for(auto& item : array.value()){
            newEnv->define(varSym->name.str(), item);
            for(size_t i = 2; i < elems.size(); ++i){
                // result = elems[i]->accept(*this, newEnv);
                [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
            }
        }
    }else if(ELux::is_hashset(listVal)){
        const HashSet& xset = *dynamic_cast<HashSet*>(listVal.get());
        for(auto& item : xset.value()) {
            newEnv->define(varSym->name.str(), std::make_shared<String>(item));
            for(size_t i = 2; i < elems.size(); ++i){
                // result = elems[i]->accept(*this, newEnv);
                [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
            }
        }
    }else if(ELux::is_hashmap(listVal)){
        const HashMap& dict = *dynamic_cast<HashMap*>(listVal.get());
        for(auto& item : dict.value()) {
            std::vector<Self> kv{};
            kv.push_back(item.first);
            kv.push_back(item.second);
            auto data = std::make_shared<Array>();
            data->value().insert(data->value().begin(), kv.begin(), kv.end());
            newEnv->define(varSym->name.str(), data);
            for(size_t i = 2; i < elems.size(); ++i){
                // result = elems[i]->accept(*this, newEnv);
                [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
            }
        }
    }else{
        const std::string& xstr = ELux::str(listVal);
        for(auto& item : xstr){
            newEnv->define(varSym->name.str(), std::make_shared<String>(item));
            for(size_t i = 2; i < elems.size(); ++i){
                // result = elems[i]->accept(*this, newEnv);
                [[maybe_unused]] auto _ = elems[i]->eval(*this, newEnv);
            }
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
size_t ModuleHash::operator()(const Module& self) const{
    auto key = self.key();
    return std::hash<std::string>{}(key);
}

// -*-
bool ModuleEqual::operator()(const Module& lhs, const Module& rhs) const{
    return (lhs.key()==rhs.key());
}

/*
// -*-
class Module final{
public:

Module::Module(ELux* elux, const Symbol name){}
Module::Module(ELux* elux, const fs::path& modulePath){}
Module::Module(Module&& other) noexcept{}
Module& Module::operator=(Module&& other) noexcept{}
const std::string& Module::key(void) const{}
std::string Module::name_from_key(const std::string& text){}
void Module::setup(void){}

private:
    Symbol m_name;              // module nmae
    fs::path m_fullpath;        // module fullpath
    std::string m_filename;     // module filename
    Context m_env;              // module environment
    ELux* m_elux;               // the interpreter


};
*/

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-