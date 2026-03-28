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
// --------------
// -*- Symbol -*-
// --------------
Symbol::Symbol(const std::string& val)
: value{val}
{}

Symbol::Symbol(const Symbol& sym) noexcept
: value{sym.value}
{}

Symbol::Symbol(Symbol&& sym) noexcept
: value{std::move(sym.value)}{
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

std::string Symbol::type(void) const{
    return "Symbol";
}

std::string Symbol::str(void) const{
    return this->value;
}

// ------------
// -*- Pair -*-
// ------------
Pair::Pair()
: key{ELux::share()}
, val{ELux::share()}
{}

Pair::Pair(Self key_)
: key{std::move(key_)}
, val{ELux::share()}
{
    key_ = nullptr;
}

// -*-
Pair::Pair(Self key_, Self val_)
: key{std::move(key_)}
, val{std::move(val_)}
{
    key_ = nullptr;
    val_ = nullptr;
}

Pair::Pair(const Pair& pair) noexcept
: key{pair.key}
, val{pair.val}
{}

Pair::Pair(Pair&& pair) noexcept
: key{std::move(pair.key)}
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
std::string Pair::type(void) const{
    return "Pair";
}

std::string Pair::str(void) const{
    std::stringstream ss;
    ss << "#(" << key->str() << " " << val->str() << ")";
    return ss.str();
}

// -------------
// -*- Tuple -*-
// -------------
Tuple::Tuple()
: items{}{}

Tuple::Tuple(const std::initializer_list<Self>& xs)
: items{Vec<Self>(xs.begin(), xs.end())}
{}

Tuple::Tuple(const Vec<Self>& xs)
: items{xs}
{}

Tuple::Tuple(const std::list<Self>& xs)
: items{Vec<Self>(xs.begin(), xs.end())}
{}

Tuple::Tuple(const Pair& xs)
: items{Vec<Self>{xs.key, xs.val}}
{}

Tuple::Tuple(const List& xs)
{
    auto data = xs.value();
    this->items = Vec<Self>(data.begin(), data.end());
}

Tuple::Tuple(const Array& xs)
: items{xs.value()}
{}

Tuple::Tuple(const Set& xs){
    auto data = xs.value();
    this->items = {};
    for(auto item: data){
        this->items.push_back(ELux::share(item));
    }
}

Tuple::Tuple(const Dict& xs){
    auto data = xs.value();
    this->items = {};
    for(auto [key, val]: data){
        auto xkey = ELux::share(key);
        auto xval = val;
        Pair pair(xkey, xval);
        this->items.push_back(ELux::share(pair));
    }
}

Tuple::Tuple(const Tuple& tuple) noexcept
: items{tuple.items}
{}

/*
struct Tuple final: public Object{


Tuple::Tuple(Tuple&& tuple) noexcept{}
Tuple& Tuple::operator=(const Tuple& tuple) noexcept{}
Tuple& Tuple::operator=(Tuple&& tuple) noexcept{}
std::string Tuple::type(void) const{}
std::string Tuple::str(void) const{}
};
// -*-
class ELuxError final: public Object, public std::runtime_error {
public:

Symbol ELuxError::ValueError;
Symbol ELuxError::TypeError;
Symbol ELuxError::SyntaxError;
Symbol ELuxError::RuntimeError;
Symbol ELuxError::KeyError;
Symbol ELuxError::IndexError;

ELuxError::ELuxError(){}
ELuxError::ELuxError(const Symbol& sym){}
ELuxError::ELuxError(const Symbol& sym, const std::string& msg){}

std::string ELuxError::describe(void) const{}
const Symbol& ELuxError::kind(void) const{}
Symbol& ELuxError::kind(void){}
std::string ELuxError::type(void) const{}
std::string ELuxError::str(void) const{}

private:
    Symbol m_kind;
};

*/

// --------------
// -*- Number -*-
// --------------
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
std::string& String::value(void){
    return this->m_val;
}

const std::string& String::value(void) const{
    return this->m_val;
}

// -----------
// -*- Set -*-
// -----------
Set::Set(): m_hset{} {}

// -*-
Set::Set(std::initializer_list<Self> xs) : m_hset{}{
    for(auto x: xs){
        this->m_hset.insert(ELux::str(x));
    }
}

// -*-
Set::Set(const Array& xs) : m_hset{}{
    for(auto x: xs.value()){
        this->m_hset.insert(ELux::str(x));
    }
}

// -*-
Set::Set(const List& xs) : m_hset{}{
    for(auto x: xs.value()){
        this->m_hset.insert(ELux::str(x));
    }
}

// -*-
Set::Set(const Set& xs) noexcept
: m_hset{xs.m_hset} {}

// -*-
Set::Set(Set&& xs) noexcept
: m_hset{std::move(xs.m_hset)}{
    xs.m_hset = {};
}

// -*-
Set& Set::operator=(const Set& xs) noexcept{
    if(this != &xs){
        this->m_hset = xs.m_hset;
    }

    return *this;
}

// -*-
Set& Set::operator=(Set&& xs) noexcept{
    if(this != &xs){
        this->m_hset = std::move(xs.m_hset);
        xs.m_hset = {};
    }

    return *this;
}

// -*-
Set::operator HSet() const{ return this->m_hset; }

std::string Set::type(void) const{
    return "Set";
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

// ------------
// -*- Dict -*-
// ------------
Dict::Dict(): m_hmap{}{}

Dict::Dict(std::initializer_list<Self> xs): m_hmap{} {
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
}

// -*-
Dict::Dict(const Array& xs): m_hmap{} {
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
}

// -*-
Dict::Dict(const List& xs): m_hmap{} {
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
}

// -*-
Dict::Dict(const Dict& xs) noexcept
: m_hmap{xs.m_hmap} {}

// -*-
Dict::Dict(Dict&& xs) noexcept
: m_hmap{std::move(xs.m_hmap)}{
    xs.m_hmap = {};
}

// -*-
Dict& Dict::operator=(const Dict& xs) noexcept{
    if(this != &xs){
        this->m_hmap = xs.m_hmap;
    }
    return *this;
}

// -*-
Dict& Dict::operator=(Dict&& xs) noexcept{
    if(this != &xs){
        this->m_hmap = std::move(xs.m_hmap);
        xs.m_hmap = {};
    }
    return *this;
}

// -*-
Dict::operator HMap() const{ return this->m_hmap; }

// -*-
std::string Dict::type(void) const{
    return "Dict";
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

// ------------
// -*- List -*-
// ------------
List::List(): m_xs{}{}

List::List(std::initializer_list<Self> xs): m_xs{}{
    for(auto self: xs){
        this->m_xs.push_back(std::move(self));
    }
}

// -*-
List::List(const Array& xs): m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::move(self));
    }
}

// -*-
List::List(const Set& xs): m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::make_shared<String>(self));
    }
}

// -*-
List::List(const Dict& xs): m_xs{}{
    for(const auto& [key, val]: static_cast<HMap>(xs)){
        auto self = std::make_shared<Array>();
        self->value().push_back(ELux::share(key));
        self->value().push_back(std::move(val));
        this->m_xs.push_back(std::move(self));
    }
}

// -*-
List::List(const List& xs) noexcept
: m_xs{xs.m_xs}{}

// -*-
List::List(List&& xs) noexcept
: m_xs{std::move(xs.m_xs)}{
    xs.m_xs = {};
}

// -*-
List& List::operator=(const List& xs) noexcept{
    if(this != &xs){
        this->m_xs = xs.m_xs;
    }
    return *this;
}

// -*-
List& List::operator=(List&& xs) noexcept{
    if(this != &xs){
        this->m_xs = std::move(xs.m_xs);
        xs.m_xs = {};
    }
    return *this;
}

// -*-
std::string List::type(void) const{
    return "List";
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

// -------------
// -*- Array -*-
// -------------
Array::Array(): m_xs{}{}

// -*-
Array::Array(std::initializer_list<Self> xs): m_xs{}{
    for(auto self: xs){
        this->m_xs.push_back(std::move(self));
    }
}

// -*-
Array::Array(const List& xs): m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::move(self));
    }
}

// // -*-
// Array::Array(const List& xs): m_xs{}{
//     for(auto self: xs.value()){
//         this->m_xs.push_back(std::move(self));
//     }
// }

// -*-
Array::Array(const Set& xs): m_xs{}{
    for(auto self: xs.value()){
        this->m_xs.push_back(std::make_shared<String>(self));
    }
}

// -*-
Array::Array(const Dict& xs): m_xs{}{
    for(auto [key, val]: xs.value()){
        auto data = std::initializer_list<Self>{
            std::make_shared<String>(key),
            std::move(val)
        };
        this->m_xs.push_back(std::make_shared<Array>(data));
    }
}

// -*-
Array::Array(const Array& xs) noexcept
: m_xs{xs.m_xs} {}

// -*-
Array::Array(Array&& xs) noexcept
: m_xs{std::move(xs.m_xs)}{}

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
std::string Array::type(void) const{
    return "Array";
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


// -*----------------------------------------------------------------*-
}//-*- end::namespace::ekasoft::klx                                 -*-
// -*----------------------------------------------------------------*-

