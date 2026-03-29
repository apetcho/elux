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
#include<limits>
#include<sstream>
#include<iomanip>
#include<cstring>
#include<cctype>
#include<cerrno>
#include<cfenv>
#include<cmath>

// -*----------------------------------------------------------------*-
// -*- begin::namespace::ekasoft::klx                               -*-
// -*----------------------------------------------------------------*-
namespace ekasoft::klx{
// -
// -----------
// -*- Nil -*-
// -----------
// -*-
Symbol Nil::type(void) const{
    return Symbol("Nil");
}

usize Nil::hash(void) const{
    return std::hash<std::string>{}("nil");
}

// -*-
bool Nil::equal(Object* other) const{
    if(this->type()==other->type()){
        return this->str()==other->str();
    }

    return false;
}

// ----------------
// -*- Iterable -*-
// ----------------
Iterable::Iterable(Object* data): m_data{data}{}

Iterator Iterable::map(Function func, Context env){
    Vec<Self> vec{};
    while(!this->done()){
        Vec<Self> args = {this->next()};
        vec.push_back(std::move(func.call(args, env)));
    }
    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::filter(Function func, Context env){
    Vec<Self> vec{};
    while(!this->done()){
        Vec<Self> args = {this->next()};
        auto ans = func.call(args, env);
        if(!ELux::is_bool(ans)){
            std::stringstream ss;
            ss << "`filter: the first argument must be a unary predicate.";
            throw std::runtime_error(ss.str());
        }

        if(ELux::as_bool(ans)){
            vec.push_back(std::move(ans));
        }
    }
    return std::make_shared<Array>(vec);
}

// -*-
Self Iterable::reduce(Function func, Context env, const Self& init){
    Self acc = init;

    while(!this->done()){
        Vec<Self> args = {acc, this->next()};
        acc = func.call(args, env);
    }

    return std::move(acc);
}

// -*-
Iterator Iterable::zip(Vec<Iterator> iterators){
    Vec<Self> vec{};
    
    while(true){
        auto stop = std::any_of(
            iterators.begin(), iterators.end(),
            [this](Iterator iter){ return iter->done(); }
        );
        if(stop){ break; }
        Vec<Self> record{};
        record.push_back(this->next());
        std::for_each(
            iterators.begin(), iterators.end(),
            [this, &record](const Iterator& iter){
                record.push_back(iter->next());
            }
        );
        Tuple tuple(record);
        vec.push_back(ELux::share(tuple));
    }

    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::chain(Vec<Iterator> iterators){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(this->next());
    }
    std::for_each(
        iterators.begin(), iterators.end(),
        [&vec](Iterator iter){
            while(!iter->done()){
                vec.push_back(std::move(iter->next()));
            }
        }
    );

    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::take(u32 n){
    Vec<Self> vec{};
    for(u32 i=0; i < n; i++){
        if(this->done()){ break; }
        vec.push_back(std::move(this->next()));
    }

    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::enumerate(void){
    Vec<Self> vec{};
    u32 idx = 0;
    while(!this->done()){
        auto key = ELux::share(static_cast<i64>(idx));
        auto val = this->next();
        auto self = ELux::share(Pair(key, val));
        vec.push_back(std::move(self));
    }

    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::drop_while(Function func, Context env){
    Vec<Self> vec{};
    while(!this->done()){
        auto self = this->next();
        auto ans = func.call(Vec<Self>{self}, env);
        if(!ELux::is_bool(ans)){
            std::stringstream ss;
            ss << "`drop-while': the first argument must be a unary predicate.";
            throw std::runtime_error(ss.str());
        }

        if(!ELux::as_bool(ans)){
            vec.push_back(std::move(self));
        }
    }

    return std::make_shared<Array>(vec);
}

// -*-
Iterator Iterable::take_while(Function func, Context env){
    Vec<Self> vec{};
    while(!this->done()){
        auto self = this->next();
        auto ans = func.call(Vec<Self>{self}, env);
        if(!ELux::is_bool(ans)){
            std::stringstream ss;
            ss << "`take-while': the first argument must be a unary predicate.";
            throw std::runtime_error(ss.str());
        }

        if(ELux::as_bool(ans)){
            vec.push_back(std::move(self));
        }
    }
    return std::make_shared<Array>(vec);
}

// -*-
bool Iterable::any(Function func, Context env){
    bool result{false};
    while(!this->done()){
        auto ans = func.call(Vec<Self>{this->next()}, env);
        if(!ELux::is_bool(ans)){
            std::stringstream ss;
            ss << "`any': the first argument must be a unary predicate.";
            throw std::runtime_error(ss.str());
        }

        if(ELux::as_bool(ans)){
            result = true;
            break;
        }
    }

    return result;
}

// -*-
bool Iterable::all(Function func, Context env){
    bool result{true};
    while(!this->done()){
        auto ans = func.call(Vec<Self>{this->next()}, env);
        if(!ELux::is_bool(ans)){
            std::stringstream ss;
            ss << "`all': the first argument must be a unary predicate.";
            throw std::runtime_error(ss.str());
        }

        if(!ELux::as_bool(ans)){
            result = false;
            break;
        }
    }

    return result;
}

// -*-
Iterator Iterable::reverse(void){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(std::move(this->next()));
    }
    vec = Vec<Self>(vec.rbegin(), vec.rend());
    return std::make_shared<Array>(vec);
}

// -*-
void Iterable::collect(String& result){
    std::stringstream ss;
    while(!this->done()){
        ss << this->next()->str();
    }
    result = String(ss.str());
}

// -*-
void Iterable::collect(Array& result){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(std::move(this->next()));
    }
    result = Array(vec);
}

// -*-
void Iterable::collect(List& result){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(std::move(this->next()));
    }
    result = List(vec);
}

void Iterable::collect(Tuple& result){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(std::move(this->next()));
    }
    result = Tuple(vec);
}

// -*-
void Iterable::collect(Set& result){
    Vec<Self> vec{};
    while(!this->done()){
        vec.push_back(std::move(this->next()));
    }
    result = Set(vec);
}

// -*-
void Iterable::collect(Dict& result){
    Vec<Pair> vec{};
    while(!this->done()){
        auto item = this->next();
        if(!ELux::is_pair(item)){
            std::stringstream ss;
            ss << "`collect(Dict&)': expect each item in the iterator to be a pair but got ";
            ss << std::quoted(item->type().str());
            throw std::runtime_error(ss.str());
        }
        auto pair = ELux::as_pair(item);
        if(!ELux::is_string(pair.key)){
            std::stringstream ss;
            ss << "`collect(Dict&)': expect each item in the iterator to be a pair but got ";
            ss << std::quoted(item->type().str()) << "\nwhere the `.key' component of each pair ";
            ss << "is a string. The type of the `.key' here is ";
            ss << std::quoted(pair.key->type().str());
            throw std::runtime_error(ss.str());
        }
        vec.push_back(std::move(pair));
    }

    result = Dict(vec);
}

// --------------
// -*- Symbol -*-
// --------------
Symbol::Symbol(const std::string& val)
: Hashable(this)
, Equalable(this)
, value{val}
{}

Symbol::Symbol(const Symbol& sym) noexcept
: Hashable(this)
, Equalable(this)
, value{sym.value}
{}

Symbol::Symbol(Symbol&& sym) noexcept
: Hashable(this)
, Equalable(this)
, value{std::move(sym.value)}{
    sym.value = {};
}

Symbol& Symbol::operator=(const Symbol& sym) noexcept{
    if(this != &sym){
        this->value = sym.value;
    }
    return *this;
}

Symbol& Symbol::operator=(Symbol&& sym) noexcept{
    if(this != &sym){
        this->value = std::move(sym.value);
        sym.value = {};
    }
    return *this;
}

Symbol Symbol::type(void) const{
    return Symbol("Symbol");
}

std::string Symbol::str(void) const{
    return this->value;
}

// -*-
usize Symbol::hash(void) const{
    return std::hash<std::string>{}(this->value);
}

// -*-
bool Symbol::equal(Object* other) const{
    if(this->type()==other->type()){
        return (this->str()==other->str());
    }

    return false;
}

// -*-
bool operator==(const Symbol& lhs, const Symbol& rhs){
    return (lhs.value==rhs.value);
}

// -*-
bool operator!=(const Symbol& lhs, const Symbol& rhs){
    return !(lhs==rhs);
}

// ------------
// -*- Pair -*-
// ------------
// -*-
Pair::Pair(Self key_, Self val_)
: Hashable(this)
, Equalable(this)
, key{std::move(key_)}
, val{std::move(val_)}
{
    key_ = nullptr;
    val_ = nullptr;
}

Pair::Pair(const Pair& pair) noexcept
: Hashable(this)
, Equalable(this)
, key{pair.key}
, val{pair.val}
{}

Pair::Pair(Pair&& pair) noexcept
: Hashable(this)
, Equalable(this)
, key{std::move(pair.key)}
, val{std::move(pair.val)}
{
    pair.key = nullptr;
    pair.val = nullptr;
}

// -*-
Pair& Pair::operator=(const Pair& pair) noexcept{
    if(this != &pair){
        this->key = pair.key;
        this->val = pair.val;
    }
    return *this;
}

Pair& Pair::operator=(Pair&& pair) noexcept{
    if(this != &pair){
        this->key = std::move(pair.key);
        this->val = std::move(pair.val);
        pair.key = nullptr;
        pair.val = nullptr;
    }
    return *this;
}

// -*-
Symbol Pair::type(void) const{
    return Symbol("Pair");
}

std::string Pair::str(void) const{
    std::stringstream ss;
    ss << "#(" << key->str() << " " << val->str() << ")";
    return ss.str();
}

// -*-
std::string Pair::repr(void) const{
    std::stringstream ss;
    ss << "#(" << key->repr() << " " << val->repr() << ")";
    return ss.str();
}

// -*-
usize Pair::hash(void) const{
    if(ELux::is_hashable(this->key) && ELux::is_hashable(this->val)){
        auto lhs = dynamic_cast<Hashable*>(this->key.get());
        auto rhs = dynamic_cast<Hashable*>(this->val.get());
        return (lhs->hash() ^ rhs->hash());
    }
    std::stringstream ss;
    ss << "calling `hash' on a pair. The two field (i.e key & val) must be hashable.";
    throw std::runtime_error(ss.str());
}

// -*-
bool Pair::equal(Object* other) const{
    if(this->type()==other->type()){
        auto rhs = dynamic_cast<Pair*>(other);
        auto ans = (ELux::is_equalable(this->key) && ELux::is_equalable(rhs->key));
        auto xeq = dynamic_cast<Equalable*>(this->key.get());
        ans = ans && xeq->equal(rhs->key.get());
        ans = (
            ans &&
            dynamic_cast<Equalable*>(this->val.get())->equal(rhs->val.get())
        );
        return ans;
    }
    return false;
}

// -------------
// -*- Tuple -*-
// -------------
Tuple::Tuple()
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{}{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const std::initializer_list<Self>& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{Vec<Self>(xs.begin(), xs.end())}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Vec<Self>& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{xs}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const std::list<Self>& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{Vec<Self>(xs.begin(), xs.end())}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Pair& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{Vec<Self>{xs.key, xs.val}}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const List& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
{
    auto data = xs.value();
    this->m_items = Vec<Self>(data.begin(), data.end());
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Array& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{xs.value()}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Set& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
{
    auto data = xs.value();
    this->m_items = {};
    for(auto item: data){
        this->m_items.push_back(ELux::share(item));
    }
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Dict& xs)
: Iterable(this)
, Hashable(this)
, Equalable(this)
{
    auto data = xs.value();
    this->m_items = {};
    for(auto [key, val]: data){
        auto xkey = ELux::share(key);
        auto xval = val;
        Pair pair(xkey, xval);
        this->m_items.push_back(ELux::share(pair));
    }
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(const Tuple& tuple) noexcept
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{tuple.m_items}
{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
}

Tuple::Tuple(Tuple&& tuple) noexcept
: Iterable(this)
, Hashable(this)
, Equalable(this)
, m_items{std::move(tuple.m_items)}{
    this->m_ptr = this->m_items.begin();
    this->m_stop = this->m_items.end();
    tuple.m_items = {};
    tuple.m_ptr = tuple.m_items.begin();
    tuple.m_stop = tuple.m_items.end();
}

Tuple& Tuple::operator=(const Tuple& tuple) noexcept{
    if(this != &tuple){
        this->m_items = tuple.m_items;
        this->m_ptr = this->m_items.begin();
        this->m_stop = this->m_items.end();
    }
    return *this;
}

Tuple& Tuple::operator=(Tuple&& tuple) noexcept{
    if(this != &tuple){
        this->m_items = std::move(tuple.m_items);
        this->m_ptr = this->m_items.begin();
        this->m_stop = this->m_items.end();
        tuple.m_items = {};
        tuple.m_ptr = tuple.m_items.begin();
        tuple.m_stop = tuple.m_items.end();
    }
    return *this;
}

Symbol Tuple::type(void) const{
    return Symbol("Tuple");
}

std::string Tuple::str(void) const{
    std::stringstream ss;
    ss << "#[";
    for(size_t i=0; i < this->m_items.size(); i++){
        if(i > 0){ ss << " "; }
        ss << this->m_items[i]->str();
    }
    ss << "]";

    return ss.str();
}

// -*-
Self Tuple::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);

    return self;
}

// -*-
bool Tuple::done(void) const{
    return (this->m_ptr==this->m_stop ? true: false);
}

// -*-
usize Tuple::hash(void) const{
    auto check = std::all_of(
        this->m_items.begin(), this->m_items.end(),
        [](const Self& self){
            return ELux::is_hashable(self);
        }
    );
    if(check){
        usize ans = std::hash<std::string>{}("Tuple");
        std::for_each(
            this->m_items.begin(), this->m_items.end(),
            [&ans](const Self& self){
                auto item = dynamic_cast<Hashable*>(self.get());
                ans = ans ^ item->hash();
            }
        );
        return ans;
    }
}

// -*-
bool Tuple::equal(Object* other) const{
    if(this->type()==other->type()){
        auto tuple = dynamic_cast<Tuple*>(other);
        if(this->len()!=tuple->len()){ return false; }
        auto myIter = ELux::as_iterator(ELux::share(*this));
        auto tupleIter = ELux::as_iterator(ELux::share(*tuple));
        auto iter = myIter->zip(Vec<Iterator>{tupleIter});
        auto ans = true;
        while(!iter->done()){
            auto vec = ELux::as_tuple(iter->next()).value();
            auto lhs = vec[0];
            auto rhs = vec[1];
            if(ELux::is_equalable(lhs) && ELux::is_equalable(rhs)){
                auto x = dynamic_cast<Equalable*>(lhs.get());
                ans = ans  && x->equal(rhs.get());
            }else{
                ans = false;
                break;
            }
        }

        return ans;
    }
    return false;
}

// -----------------
// -*- ELuxError -*-
// -----------------
Symbol ELuxError::ValueError = Symbol("ValueError");
Symbol ELuxError::TypeError = Symbol("TypeError");
Symbol ELuxError::SyntaxError = Symbol("SyntaxError");
Symbol ELuxError::RuntimeError = Symbol("RuntimeError");
Symbol ELuxError::KeyError = Symbol("KeyError");
Symbol ELuxError::IndexError = Symbol("IndexError");

// -*-
ELuxError::ELuxError()
: Hashable(this)
, Equalable(this)
, m_kind{Symbol("Error")}
, m_msg{"unexpected error caught."}
{}

// -*-
ELuxError::ELuxError(const std::string& msg)
: Hashable(this)
, Equalable(this)
, m_kind{Symbol("Error")}
, m_msg{msg}
{}

// -*-
ELuxError::ELuxError(const Symbol& sym)
: Hashable(this)
, Equalable(this)
, m_kind{sym}
, m_msg{"unexpected error caught."}
{}

ELuxError::ELuxError(const Symbol& sym, const std::string& msg)
: Hashable(this)
, Equalable(this)
, m_kind{sym}
, m_msg{msg}
{}

// -*-
ELuxError::ELuxError(const ELuxError& err) noexcept
: Hashable(this)
, Equalable(this)
, m_kind{err.m_kind}
, m_msg{err.m_msg}
{}

// -*-
ELuxError::ELuxError(ELuxError&& err) noexcept
: Hashable(this)
, Equalable(this)
, m_kind{std::move(err.m_kind)}
, m_msg{std::move(err.m_msg)}
{
    err.m_msg = {};
}

// -*-
ELuxError& ELuxError::operator=(const ELuxError& err) noexcept{
    if(this != &err){
        this->m_kind = err.m_kind;
        this->m_msg = err.m_msg;
    }

    return *this;
}

// -*-
ELuxError& ELuxError::operator=(ELuxError&& err) noexcept{
    if(this != &err){
        this->m_kind = std::move(err.m_kind);
        this->m_msg = std::move(err.m_msg);
        err.m_msg = {};
    }

    return *this;
}

// -*-
std::string ELuxError::describe(void) const{
    std::stringstream ss;
    ss << this->m_kind.str() << ": " << this->m_msg;
    return ss.str();
}

// -*-
const Symbol& ELuxError::kind(void) const{
    return this->m_kind;
}

Symbol& ELuxError::kind(void){
    return this->m_kind;
}

Symbol ELuxError::type(void) const{
    return this->kind();
}

std::string ELuxError::str(void) const{
    return std::string(this->m_msg);
}

// -*-
usize ELuxError::hash(void) const{
    auto x = this->m_kind.hash();
    auto y = std::hash<std::string>{}(this->m_msg);
    return (x ^ y);
}

// -*-
bool ELuxError::equal(Object* other) const{
    if(this->type()==other->type()){
        auto rhs = dynamic_cast<ELuxError*>(other);
        auto x = this->m_kind.equal(rhs);
        auto y = (this->m_msg==rhs->m_msg);
        return (x && y);
    }
    return false;
}

// ------------
// -*- Bool -*-
// ------------
// -*-
Bool& Bool::operator=(const Bool& other) noexcept{
    if(this != &other){
        this->m_val = other.m_val;
    }
    return *this;
}

// -*-
Bool& Bool::operator=(Bool&& other) noexcept{
    if(this != &other){
        this->m_val = std::move(other.m_val);
    }
    return *this;
}

// -*-
Symbol Bool::type(void) const{
    return Symbol("Bool");
}

// --------------
// -*- Number -*-
// --------------
Number::Number(const Number& num) noexcept
: Hashable(this)
, TotalOrdering(this)
, m_val{num.m_val}{}

// -*-
Number::Number(Number&& num) noexcept
: Hashable(this)
, TotalOrdering(this)
, m_val{std::move(num.m_val)}{}

// -*-
Number& Number::operator=(const Number& num) noexcept{
    if(this != &num){
        this->m_val = num.m_val;
    }
    return *this;
}

// -*-
Number& Number::operator=(Number&& num) noexcept{
    if(this != &num){
        this->m_val = std::move(num.m_val);
    }
    return *this;
}

// -*-
usize Number::hash(void) const{
    if(this->is_integer()){
        return std::hash<i64>{}(this->as_integer());
    }
    std::stringstream ss;
    ss << "hashing floating point number is not supported because of precision errors.";
    throw std::runtime_error(ss.str());
}

// -*-
bool Number::equal(Object* obj) const{
    auto self = dynamic_cast<Number*>(obj);
    if(self == nullptr){ return false; }
    auto rhs = *dynamic_cast<Number*>(obj);
    return (*this==rhs);
}

// -*-
int Number::compare(Object* obj) const{
    auto self = dynamic_cast<Number*>(obj);
    if(self==nullptr){
        std::stringstream ss;
        ss << "cannot compare " << std::quoted(obj->type().str()) << " to numbers.";
        throw std::runtime_error(ss.str());
    }
    auto rhs = *self;
    if(*this < rhs){ return -1; }
    if(*this > rhs){ return 1; }
    return 0;
}

// -*-
Symbol Number::type(void) const{
    if(this->is_integer()){ return Symbol("Integer"); }
    return Symbol("Float");
}

// -*-
bool Number::as_bool(void) const{
    auto result = static_cast<bool>(*this);
    return result;
}

i64 Number::as_integer(void) const{
    if(this->is_integer()){
        return std::get<i64>(this->m_val);
    }
    auto num = std::get<f64>(this->m_val);
    return static_cast<i64>(num);
}

// -*-
f64 Number::as_float(void) const{
    if(this->is_integer()){
        auto num = std::get<i64>(this->m_val);
        return static_cast<f64>(num);
    }

    return std::get<f64>(this->m_val);
}

// -*-
Number Number::abs(void) const{
    if(this->is_integer()){
        auto num = this->as_integer();
        num = (num < 0 ? -num : num);
        return Number(num);
    }

    auto num = this->as_float();
    return Number(std::fabs(num));
}

// -*-
Number Number::ceil(void) const{
    return Number(std::ceil(this->as_float()));
}

// -*-
Number Number::floor(void) const{
    return Number(std::floor(this->as_float()));
}

// -*-
Number Number::round(void) const{
    return Number(std::round(this->as_float()));
}

// -*-
Number Number::truncate(void) const{
    return Number(std::trunc(this->as_float()));
}

// -*-
Number Number::sin(void) const{
    return Number(std::sin(this->as_float()));
}

// -*-
Number Number::cos(void) const{
    return Number(std::cos(this->as_float()));
}

// -*-
Number Number::tan(void) const{
    return Number(std::tan(this->as_float()));
}

// -*-
Number Number::asin(void) const{
    return Number(std::asin(this->as_float()));
}

// -*-
Number Number::acos(void) const{
    return Number(std::acos(this->as_float()));
}

// -*-
Number Number::atan(void) const{
    return Number(std::atan(this->as_float()));
}

// -*-
Number Number::atan2(const Number& rhs) const{
    auto y = this->as_float();
    auto x = rhs.as_float();
    f64 num{};
    try{
        num = std::atan2(y, x);
    }catch(const std::exception& err){
        throw std::runtime_error(err.what());
    }
    return Number(num);
}

// -*-
Number Number::sinh(void) const{
    return Number(std::sinh(this->as_float()));
}

// -*-
Number Number::cosh(void) const{
    return Number(std::cosh(this->as_float()));
}

// -*-
Number Number::tanh(void) const{
    return Number(std::tanh(this->as_float()));
}

// -*-
Number Number::asinh(void) const{
    return Number(std::asinh(this->as_float()));
}

// -*-
Number Number::acosh(void) const{
    return Number(std::acosh(this->as_float()));
}

// -*-
Number Number::atanh(void) const{
    return Number(std::atanh(this->as_float()));
}

// -*-
Number Number::exp(void) const{
    return Number(std::exp(this->as_float()));
}

// -*-
Number Number::expm1(void) const{
    return Number(std::expm1(this->as_float()));
}

// -*-
Number Number::exp2(void) const{
    return Number(std::exp2(this->as_float()));
}

// -*-
Number Number::pow(const Number& rhs) const{
    auto base = this->as_float();
    auto expo = rhs.as_float();
    auto num = std::pow(base, expo);
    return Number(num);
}

// -*-
Number Number::sqrt(void) const{
    return Number(std::sqrt(this->as_float()));
}

// -*-
Number Number::cbrt(void) const{
    return Number(std::cbrt(this->as_float()));
}

// -*-
Number Number::log(void) const{
    return Number(std::log(this->as_float()));
}

// -*-
Number Number::log2(void) const{
    return Number(std::log2(this->as_float()));
}

// -*-
Number Number::log10(void) const{
    return Number(std::log10(this->as_float()));
}

// -*-
Number Number::log1p(void) const{
    return Number(std::log1p(this->as_float()));
}

// -*-
Number Number::erf(void) const{
    return Number(std::erf(this->as_float()));
}

// -*-
Number Number::erfc(void) const{
    return Number(std::erfc(this->as_float()));
}

// -*-
Number Number::tgamma(void) const{
    return Number(std::tgamma(this->as_float()));
}

// -*-
Number Number::lgamma(void) const{
    return Number(std::lgamma(this->as_float()));
}

bool Number::isnan(void) const{
    return std::isnan(this->as_float());
}

// -*-
bool Number::isinf(void) const{
    return std::isinf(this->as_float());
}

// -*-
bool Number::isfinite(void) const{
    return std::isfinite(this->as_float());
}

// -*-
Number& Number::operator!(){
    auto flag = static_cast<bool>(*this);
    i64 num = (flag ? 0 : 1);
    this->m_val = num;
    return *this;
}

// -*-
Number& Number::operator-(){
    if(this->is_integer()){
        auto num = this->as_integer();
        this->m_val = -num;
    }else{
        auto num = this->as_float();
        this->m_val = -num;
    }
    return *this;
}

// -*-
Number& Number::operator~(){
    if(!this->is_integer()){
        std::stringstream ss;
        ss << "`~' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto num = ~this->as_integer();
    this->m_val = num;
    return *this;
}

// -*-
Number operator+(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return Number((x+y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return Number((x+y));
}

// -*-
Number operator-(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return Number((x-y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return Number((x-y));
}

// -*-
Number operator*(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return Number((x*y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return Number((x*y));
}

// -*-
Number operator/(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        if(y==0){
            throw std::runtime_error("division by zero");
        }
        return Number((x/y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    if(y==0.0){
        throw std::runtime_error("division by zero");
    }
    return Number((x/y));
}

// -*-
Number operator%(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        if(y==0){
            throw std::runtime_error("division by zero");
        }
        return Number((x%y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    if(y==0.0){
        throw std::runtime_error("division by zero");
    }
    return Number(std::fmod(x, y));
}

// -*-
Number operator&(const Number& lhs, const Number& rhs){
    if(!(lhs.is_integer() && rhs.is_integer())){
        std::stringstream ss;
        ss << "`&' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto x = lhs.as_integer();
    auto y = rhs.as_integer();
    return Number((x & y));
}

// -*-
Number operator|(const Number& lhs, const Number& rhs){
    if(!(lhs.is_integer() && rhs.is_integer())){
        std::stringstream ss;
        ss << "`|' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto x = lhs.as_integer();
    auto y = rhs.as_integer();
    return Number((x | y));
}

// -*-
Number operator^(const Number& lhs, const Number& rhs){
    if(!(lhs.is_integer() && rhs.is_integer())){
        std::stringstream ss;
        ss << "`^' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto x = lhs.as_integer();
    auto y = rhs.as_integer();
    return Number((x ^ y));
}

// -*-
Number operator<<(const Number& lhs, const Number& rhs){
    if(!(lhs.is_integer() && rhs.is_integer())){
        std::stringstream ss;
        ss << "`<<' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto x = lhs.as_integer();
    auto y = rhs.as_integer();
    return Number((x << y));
}

// -*-
Number operator>>(const Number& lhs, const Number& rhs){
    if(!(lhs.is_integer() && rhs.is_integer())){
        std::stringstream ss;
        ss << "`>>' is only applicable to integers.";
        throw std::runtime_error(ss.str());
    }
    auto x = lhs.as_integer();
    auto y = rhs.as_integer();
    return Number((x >> y));
}

// -*-
bool operator&&(const Number& lhs, const Number& rhs){
    return (lhs.as_bool() && rhs.as_bool());
}

// -*-
bool operator||(const Number& lhs, const Number& rhs){
    return (lhs.as_bool() || rhs.as_bool());
}

// -*-
bool operator==(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return (x==y);
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return (x==y);
}

bool operator!=(const Number& lhs, const Number& rhs){
    return !(lhs == rhs);
}

// -*-
bool operator<(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return (x<y);
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return (x<y);
}

// -*-
bool operator>(const Number& lhs, const Number& rhs){
    return !(lhs <= rhs);
}

// -*-
bool operator<=(const Number& lhs, const Number& rhs){
    return ((lhs < rhs) || (lhs == rhs));
}

// -*-
bool operator>=(const Number& lhs, const Number& rhs){
    return !(lhs < rhs);
}

// --------------
// -*- String -*-
// --------------
// -*-
String::String()
: Iterable(this)
, Hashable(this)
, TotalOrdering(this)
, m_val{}
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
}

// -*-
String::String(const std::string& str)
: Iterable(this)
, Hashable(this)
, TotalOrdering(this)
, m_val{str}
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
}

// -*-
String::String(const char* cstr)
: Iterable(this)
, Hashable(this)
, TotalOrdering(this)
, m_val{std::string(cstr)}
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
}

// -*-
String::String(char c)
: Iterable(this)
, Hashable(this)
, TotalOrdering(this)
, m_val{std::string(1, c)}
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
}

String::String(const String& xs)
: Iterable(this)
, Hashable(this)
, TotalOrdering(this)
, m_val(xs.m_val)
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
}

// -*-
String::String(String&& xs)
: Iterable{this}
, Hashable(this)
, TotalOrdering(this)
, m_val{xs.m_val}
{
    this->m_ptr = this->m_val.begin();
    this->m_stop = this->m_val.end();
    xs.m_val = {};
    xs.m_ptr = xs.m_val.begin();
    xs.m_stop = xs.m_val.end();
}

// -*-
Self String::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);
    return ELux::share(self);
}

// -*-
bool String::done(void) const{
    return (this->m_ptr==this->m_stop? true : false);
}

// -*-
usize String::hash(void) const{
    return std::hash<std::string>{}(this->m_val);
}

// -*-
bool String::equal(Object* other) const{
    auto self = dynamic_cast<String*>(other);
    if(self==nullptr){
        return false;
    }
    auto rhs = self->m_val;
    return (this->m_val==rhs);
}


// -*-
int String::compare(Object* other) const{
    auto self = dynamic_cast<String*>(other);
    if(self==nullptr){
        std::stringstream ss;
        ss << "cannot compare " << std::quoted(other->str()) << " object to string.";
        throw std::runtime_error(ss.str());
    }

    return this->m_val.compare(self->m_val);
}

// -*-
Symbol String::type(void) const{
    return Symbol("String");
}

// -*-
std::string String::str(void) const{
    std::stringstream ss;
    ss << std::quoted(this->m_val);
    return ss.str();
}

std::string& String::value(void){
    return this->m_val;
}

const std::string& String::value(void) const{
    return this->m_val;
}

// -----------
// -*- Set -*-
// -----------
Set::Set()
: Iterable(this)
, m_hset{} {
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
}

// -*-
Set::Set(std::initializer_list<Self> xs)
: Iterable(this)
, m_hset{}{
    for(auto x: xs){
        this->m_hset.insert(ELux::str(x));
    }
}

// -*-
Set::Set(const Array& xs)
: Iterable(this)
, m_hset{}{
    for(auto x: xs.value()){
        this->m_hset.insert(ELux::str(x));
    }
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
}

// -*-
Set::Set(const List& xs)
: Iterable(this)
, m_hset{}{
    for(auto x: xs.value()){
        this->m_hset.insert(ELux::str(x));
    }
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
}

// -*-
Set::Set(const Vec<Self>& xs)
: Iterable(this)
, m_hset{}{
    for(auto x: xs){
        this->m_hset.insert(ELux::str(x));
    }
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
}

// -*-
Set::Set(const Set& xs) noexcept
: Iterable(this)
, m_hset{xs.m_hset} {
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
}

// -*-
Set::Set(Set&& xs) noexcept
: Iterable(this)
, m_hset{std::move(xs.m_hset)}{
    this->m_ptr = this->m_hset.begin();
    this->m_stop = this->m_hset.end();
    xs.m_hset = {};
    xs.m_ptr = xs.m_hset.begin();
    xs.m_stop = xs.m_hset.end();
}

// -*-
Set& Set::operator=(const Set& xs) noexcept{
    if(this != &xs){
        this->m_hset = xs.m_hset;
        this->m_ptr = this->m_hset.begin();
        this->m_stop = this->m_hset.end();
    }

    return *this;
}

// -*-
Set& Set::operator=(Set&& xs) noexcept{
    if(this != &xs){
        this->m_hset = std::move(xs.m_hset);
        this->m_ptr = this->m_hset.begin();
        this->m_stop = this->m_hset.end();
        xs.m_hset = {};
        xs.m_ptr = xs.m_hset.begin();
        xs.m_stop = xs.m_hset.end();
    }

    return *this;
}

// -*-
Set::operator HSet() const{ return this->m_hset; }

Symbol Set::type(void) const{
    return Symbol("Set");
}

// -*-
std::string Set::str(void) const{
    std::stringstream ss;
    ss << "#{";
    size_t idx = 0;
    for(auto self: this->m_hset){
        if(idx > 0){ ss << " "; }
        ss << self;
        ++idx;
    }
    ss << "}";
    return ss.str();
}

// -*-
HSet& Set::value(void){ return this->m_hset; }

// -*-
const HSet& Set::value(void) const{ return this->m_hset; }

// -*-
Self Set::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);
    return ELux::share(self);
}

// -*-
bool Set::done(void) const{
    return (this->m_ptr==this->m_stop ? true: false);
}

// ------------
// -*- Dict -*-
// ------------
Dict::Dict()
: Iterable(this)
, m_hmap{}{
    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

Dict::Dict(std::initializer_list<Self> xs)
: Iterable(this)
, m_hmap{} {
    for(auto self: xs){
        if(auto pair=dynamic_cast<List*>(self.get())){
            if(pair->value().size()==2){
                auto ptr = (pair->value().begin());
                auto key = ELux::str(*ptr);
                auto val = *(std::next(ptr));
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<Array*>(self.get())){
            if(pair->value().size()==2){
                auto key = ELux::str(pair->value()[0]);
                auto val = pair->value()[1];
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<std::pair<Self, Self>*>(self.get())){
            auto key = ELux::str(pair->first);
            auto val = pair->second;
            this->m_hmap[key] = std::move(val);
        }else{
            throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
        }
    }

    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

// -*-
Dict::Dict(const Array& xs)
: Iterable(this)
, m_hmap{} {
    for(auto self: xs.value()){
        if(auto pair=dynamic_cast<List*>(self.get())){
            if(pair->value().size()==2){
                auto ptr = (pair->value().begin());
                auto key = ELux::str(*ptr);
                auto val = *(std::next(ptr));
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<Array*>(self.get())){
            if(pair->value().size()==2){
                auto key = ELux::str(pair->value()[0]);
                auto val = pair->value()[1];
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<std::pair<Self, Self>*>(self.get())){
            auto key = ELux::str(pair->first);
            auto val = pair->second;
            this->m_hmap[key] = std::move(val);
        }else{
            throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
        }
    }

    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

// -*-
Dict::Dict(const List& xs)
: Iterable(this)
, m_hmap{} {
    for(auto self: xs.value()){
        if(auto pair=dynamic_cast<List*>(self.get())){
            if(pair->value().size()==2){
                auto ptr = (pair->value().begin());
                auto key = ELux::str(*ptr);// ELux::as_string(*ptr);
                auto val = *(std::next(ptr));
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<Array*>(self.get())){
            if(pair->value().size()==2){
                auto key = ELux::str(pair->value()[0]);
                auto val = pair->value()[1];
                this->m_hmap[key] = std::move(val);
            }else{
                throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
            }
        }else if(auto pair=dynamic_cast<std::pair<Self, Self>*>(self.get())){
            auto key = ELux::str(pair->first);
            auto val = pair->second;
            this->m_hmap[key] = std::move(val);
        }else{
            throw std::runtime_error("Dict: expect a key/value pair in the initializer_list");
        }
    }

    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

// -*-
Dict::Dict(const Vec<Pair>& pairs)
: Iterable(this)
{
    this->m_hmap = {};
    for(const auto& pair: pairs){
        auto key = ELux::str(pair.key);
        auto val = pair.val;
        this->m_hmap[key] = std::move(val);
    }
    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

// -*-
Dict::Dict(const Dict& xs) noexcept
: Iterable(this)
, m_hmap{xs.m_hmap} {
    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
}

// -*-
Dict::Dict(Dict&& xs) noexcept
: Iterable(this)
, m_hmap{std::move(xs.m_hmap)}{
    this->m_ptr = this->m_hmap.begin();
    this->m_stop = this->m_hmap.end();
    xs.m_hmap = {};
    xs.m_ptr = xs.m_hmap.begin();
    xs.m_stop = xs.m_hmap.end();
}

// -*-
Dict& Dict::operator=(const Dict& xs) noexcept{
    if(this != &xs){
        this->m_hmap = xs.m_hmap;
        this->m_ptr = this->m_hmap.begin();
        this->m_stop = this->m_hmap.end();
    }
    return *this;
}

// -*-
Dict& Dict::operator=(Dict&& xs) noexcept{
    if(this != &xs){
        this->m_hmap = std::move(xs.m_hmap);
        this->m_ptr = this->m_hmap.begin();
        this->m_stop = this->m_hmap.end();
        xs.m_hmap = {};
        xs.m_ptr = xs.m_hmap.begin();
        xs.m_stop = xs.m_hmap.end();
    }
    return *this;
}

// -*-
Dict::operator HMap() const{ return this->m_hmap; }

// -*-
Symbol Dict::type(void) const{
    return Symbol("Dict");
}

// -*-
std::string Dict::str(void) const {
    std::stringstream ss;
    ss << "{";
    size_t idx = 0;
    for(auto& [key, val]: this->m_hmap){
        if(idx > 0){ ss << " "; }
        ss << "[" << key << " " << val->str() << "]";
        ++idx;
    }
    ss << "}";
    return ss.str();
}

// -*-
HMap& Dict::value(void){ return this->m_hmap; }

const HMap& Dict::value(void) const{ return this->m_hmap; }

// -*-
Self Dict::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);
    auto key = ELux::share(self.first);
    auto val = std::move(self.second);

    return ELux::share(Pair(key, val));
}

// -*-
bool Dict::done(void) const{
    return (this->m_ptr==this->m_stop ? true : false);
}

// ------------
// -*- List -*-
// ------------
List::List()
: Iterable(this)
, m_xs{}{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

List::List(std::initializer_list<Self> xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs){
        this->m_xs.push_back(std::move(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(const Array& xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::move(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(const Set& xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::make_shared<String>(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(const Dict& xs)
: Iterable(this)
, m_xs{}{
    for(const auto& [key, val]: static_cast<HMap>(xs)){
        auto self = std::make_shared<Array>();
        self->value().push_back(ELux::share(key));
        self->value().push_back(std::move(val));
        this->m_xs.push_back(std::move(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(const Vec<Self>& xs)
: Iterable(this)
, m_xs{std::list<Self>(xs.begin(), xs.end())}
{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(const List& xs) noexcept
: Iterable(this)
, m_xs{xs.m_xs}{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
List::List(List&& xs) noexcept
: Iterable(this)
, m_xs{std::move(xs.m_xs)}{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
    xs.m_xs = {};
    xs.m_ptr = xs.m_xs.begin();
    xs.m_stop = xs.m_xs.end();
}

// -*-
List& List::operator=(const List& xs) noexcept{
    if(this != &xs){
        this->m_xs = xs.m_xs;
        this->m_ptr = this->m_xs.begin();
        this->m_stop = this->m_xs.end();
    }
    return *this;
}

// -*-
List& List::operator=(List&& xs) noexcept{
    if(this != &xs){
        this->m_xs = std::move(xs.m_xs);
        this->m_ptr = this->m_xs.begin();
        this->m_stop = this->m_xs.end();
        xs.m_xs = {};
        xs.m_ptr = xs.m_xs.begin();
        xs.m_stop = xs.m_xs.end();
    }
    return *this;
}

// -*-
Symbol List::type(void) const{
    return Symbol("List");
}

// -*-
std::string List::str(void) const{
    std::stringstream ss;
    ss << "(";
    size_t idx = 0;
    for(auto self: this->m_xs){
        if(idx > 0){ ss << " "; }
        ss << self->str();
        ++idx;
    }
    ss << ")";
    return ss.str();
}

// -*-
std::list<Self>& List::value(void){
    return this->m_xs;
}

// -*-
const std::list<Self>& List::value(void) const{
    return this->m_xs;
}

// -*-
Self List::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);
    return self;
}

// -*-
bool List::done(void) const{
    return (this->m_ptr == this->m_stop ? true : false);
}

// -------------
// -*- Array -*-
// -------------
Array::Array()
: Iterable(this)
, m_xs{}{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(std::initializer_list<Self> xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs){
        this->m_xs.push_back(std::move(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(const List& xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::move(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(const Set& xs)
: Iterable(this)
, m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::make_shared<String>(self));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(const Dict& xs)
: Iterable(this)
, m_xs{}{
    for(auto [key, val]: xs.value()){
        auto data = std::initializer_list<Self>{
            std::make_shared<String>(key),
            std::move(val)
        };
        this->m_xs.push_back(std::make_shared<Array>(data));
    }
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(const Vec<Self>& xs)
: Iterable(this)
, m_xs{xs}
{
    this->m_ptr = this->m_xs.begin();
    this->m_stop = this->m_xs.end();
}

// -*-
Array::Array(const Array& xs) noexcept
: Iterable(this)
, m_xs{xs.m_xs} {}

// -*-
Array::Array(Array&& xs) noexcept
: Iterable(this)
, m_xs{std::move(xs.m_xs)}{}

// -*-
Array& Array::operator=(const Array& xs) noexcept{
    if(this != &xs){
        this->m_xs = xs.m_xs;
    }
    return *this;
}

// -*-
Array& Array::operator=(Array&& xs) noexcept{
    if(this != &xs){
        this->m_xs = std::move(xs.m_xs);
        xs.m_xs = {};
    }
    return *this;
}

// -*-
Symbol Array::type(void) const{
    return Symbol("Array");
}

// -*-
std::string Array::str(void) const{
    std::stringstream ss;
    ss << "[";
    size_t idx = 0;
    for(const auto& self: this->m_xs){
        if(idx > 0){ ss << " "; }
        ss << self->str();
        ++idx;
    }
    ss << "]";
    return ss.str();
}

// -*-
Vec<Self>& Array::value(void){
    return this->m_xs;
}

// -*-
const Vec<Self>& Array::value(void) const{
    return this->m_xs;
}

// -*-
Self Array::next(void){
    auto self = *this->m_ptr;
    this->m_ptr = std::next(this->m_ptr);
    return self;
}

// -*-
bool Array::done(void) const{
    return (this->m_ptr==this->m_stop ? true : false);
}

// ----------------
// -*- Function -*-
// ----------------
Self Function::call(const Vec<Self>& args, Context env){
    // check argc
    if(args.size() != this->params.size()){
        std::stringstream ss;
        ss << (this->isMacro ? "macro " : "function ");
        ss << "arg count mismatch";
        throw std::runtime_error(ss.str());
    }
    // (1) builtin-function
    if(this->isNative){
        return this->native(args, env);
    }
    auto callEnv = std::make_shared<Env>(this->closure);
    for(size_t i = 0; i < this->params.size(); ++i){
        callEnv->define(this->params[i], args[i]);
    }

    // (2) macro
    if(this->isMacro){
        auto expr = this->expand(args, callEnv);
        return this->elux->eval(expr, callEnv);
    }
    
    // (3) user-defined function
    return this->body->eval(*this->elux, callEnv);
}

// -*-
Expr Function::expand(const Vec<Self>& args, Context env){
    auto self = this->body->eval(*this->elux, env);
    // macro body returns Value representing code
    return this->elux->to_expr(self);
}

// -*-
Symbol Function::type(void) const{
    if(this->isMacro){ return Symbol("Macro"); }
    if(this->isNative){ return Symbol("Builtin-Function"); }
    return Symbol("Function");
}

// -------------------
// -*- HashHandler -*-
// -------------------
usize HashHandler::operator()(const Self& self) const{
    if(!ELux::is_hashable(self)){
        std::stringstream ss;
        ss << "hash function is not supported for " << std::quoted(self->type().str());
        ss << " objects.";
        throw ELuxError(ELuxError::RuntimeError, ss.str());
    }
    auto hasher = dynamic_cast<Hashable*>(self.get());
    return hasher->hash();
}

// -*-
bool EqualHandler::operator()(const Self& lhs, const Self& rhs) const{
    if(ELux::is_equalable(lhs) && ELux::is_equalable(rhs)){
        auto x = dynamic_cast<Equalable*>(lhs.get());
        return x->equal(rhs.get());
    }

    std::stringstream ss;
    ss << "" << std::quoted(lhs->type().str());
    ss << " and " << std::quoted(rhs->type().str());
    ss << " must support equality (i.e `=') operations.";
    throw ELuxError(ELuxError::RuntimeError, ss.str());
}

// -*-
Self operator==(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        return ELux::share(xnum==ynum);
    }

    if(lhs->type() != rhs->type()){ return ELux::share(false); }
    if(ELux::is_nil(lhs)){ return ELux::share(true); }
    if(ELux::is_string(lhs)){
        auto xstr = *dynamic_cast<String*>(lhs.get());
        auto ystr = *dynamic_cast<String*>(rhs.get());
        return ELux::share(xstr.value()==ystr.value());
    }
    if(auto self=dynamic_cast<Symbol*>(lhs.get())){
        auto xsym = *self;
        auto ysym = *dynamic_cast<Symbol*>(rhs.get());
        return ELux::share(xsym.str()==ysym.str());
    }
    std::stringstream ss;
    ss << "`=' is not support for type " << std::quoted(lhs->type().str());
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator!=(const Self& lhs, const Self& rhs){
    return ELux::share(!(lhs==rhs));
}

// -*-
Self operator<=(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        return ELux::share(xnum<=ynum);
    }
    if(lhs->type()==rhs->type() && ELux::is_string(lhs)){
        auto xstr = *dynamic_cast<String*>(lhs.get());
        return ELux::share(xstr.compare(rhs.get()) < 0 || xstr.equal(rhs.get()));
    }

    std::stringstream ss;
    ss << "`<=' is not support for type " << std::quoted(lhs->type().str());
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator>=(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        return ELux::share(xnum>=ynum);
    }
    if(lhs->type()==rhs->type() && ELux::is_string(lhs)){
        auto xstr = *dynamic_cast<String*>(lhs.get());
        return ELux::share(xstr.compare(rhs.get()) > 0 || xstr.equal(rhs.get()));
    }

    std::stringstream ss;
    ss << "`>=' is not support for type " << std::quoted(lhs->type().str());
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator<(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        return ELux::share(xnum < ynum);
    }
    if(lhs->type()==rhs->type() && ELux::is_string(lhs)){
        auto xstr = *dynamic_cast<String*>(lhs.get());
        return ELux::share(xstr.compare(rhs.get()) < 0);
    }

    std::stringstream ss;
    ss << "`<' is not support for type " << std::quoted(lhs->type().str());
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator>(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        return ELux::share(xnum > ynum);
    }
    if(lhs->type()==rhs->type() && ELux::is_string(lhs)){
        auto xstr = *dynamic_cast<String*>(lhs.get());
        return ELux::share(xstr.compare(rhs.get()) > 0);
    }

    std::stringstream ss;
    ss << "`>' is not support for type " << std::quoted(lhs->type().str());
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator+(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum + ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`+' is only supported for numbers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator-(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum - ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`-' is only supported for numbers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator*(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum * ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`*' is only supported for numbers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator/(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum / ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`/' is only supported for numbers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator%(const Self& lhs, const Self& rhs){
    if(ELux::is_number(lhs) && ELux::is_number(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum % ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`%' is only supported for numbers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator<<(const Self& lhs, const Self& rhs){
    if(ELux::is_integer(lhs) && ELux::is_integer(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum << ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`<<' is only supported for integers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator>>(const Self& lhs, const Self& rhs){
    if(ELux::is_integer(lhs) && ELux::is_integer(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum >> ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`>>' is only supported for integers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator&(const Self& lhs, const Self& rhs){
    if(ELux::is_integer(lhs) && ELux::is_integer(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum & ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`&' is only supported for integers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator|(const Self& lhs, const Self& rhs){
    if(ELux::is_integer(lhs) && ELux::is_integer(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum | ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`|' is only supported for integers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator^(const Self& lhs, const Self& rhs){
    if(ELux::is_integer(lhs) && ELux::is_integer(rhs)){
        auto xnum = *dynamic_cast<Number*>(lhs.get());
        auto ynum = *dynamic_cast<Number*>(rhs.get());
        auto ans = (xnum ^ ynum);
        return ELux::share(ans);
    }

    std::stringstream ss;
    ss << "`^' is only supported for integers.";
    throw ELuxError(ELuxError::TypeError, ss.str());
}

// -*-
Self operator||(const Self& lhs, const Self& rhs){
    auto x = ELux::as_bool(lhs);
    auto y = ELux::as_bool(rhs);
    return ELux::share(x || y);
}

// -*-
Self operator&&(const Self& lhs, const Self& rhs){
    auto x = ELux::as_bool(lhs);
    auto y = ELux::as_bool(rhs);
    return ELux::share(x && y);
}

// -*----------------------------------------------------------------*-
}//-*- end::namespace::ekasoft::klx                                 -*-
// -*----------------------------------------------------------------*-

