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
// -*- Number -*-
// --------------
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

/*
// -*-
Number Number::tgamma(void) const{}
Number Number::lgamma(void) const{}
bool Number::isnan(void) const{}
bool Number::isinf(void) const{}
bool Number::isfinite(void) const{}
Number& Number::operator!(){}
Number& Number::operator-(){}
Number& Number::operator~(){}

Number operator+(const Number& lhs, const Number& rhs){}
Number operator-(const Number& lhs, const Number& rhs){}
Number operator*(const Number& lhs, const Number& rhs){}
Number operator/(const Number& lhs, const Number& rhs){}
Number operator%(const Number& lhs, const Number& rhs){}

Number operator&(const Number& lhs, const Number& rhs){}
Number operator|(const Number& lhs, const Number& rhs){}
Number operator^(const Number& lhs, const Number& rhs){}
Number operator<<(const Number& lhs, const Number& rhs){}
Number operator>>(const Number& lhs, const Number& rhs){}

bool operator&&(const Number& lhs, const Number& rhs){}
bool operator&&(const Number& lhs, const Number& rhs){}

bool operator==(const Number& lhs, const Number& rhs){}
bool operator!=(const Number& lhs, const Number& rhs){}
bool operator<(const Number& lhs, const Number& rhs){}
bool operator>(const Number& lhs, const Number& rhs){}
bool operator<=(const Number& lhs, const Number& rhs){}
bool operator>=(const Number& lhs, const Number& rhs){}

*/

// -*-
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

