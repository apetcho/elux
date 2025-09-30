#include "lynx.hpp"
#include<limits>
#include<sstream>
#include<iomanip>

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace klx{
// -

// ------------------------------
// -*- Object: the base class -*-
// ------------------------------
Symbol Object::type(void) const{
    return Symbol("nil");
}

// ------------
// -*- Bool -*-
// ------------
Bool::Bool() noexcept
: m_value{false}
{}

Bool::Bool(bool val) noexcept
: m_value{val}
{}

// -*-
Symbol Bool::type(void) const{
    return Symbol("bool");
}

// -*-
Str Bool::str(void) const{
    return this->m_value ? "true" : "false";
}

// -*-
Str Bool::repr(void) const{
    return this->m_value ? "true" : "false";
}

// -*-
bool Bool::as_bool(void) const{
    return this->m_value;
}

// -*-
i64 Bool::as_integer(void) const{
    return (
        this->m_value ?
        static_cast<i64>(1) :
        static_cast<i64>(0)
    );
}

// -*-
f64 Bool::as_float(void) const{
    return (
        this->m_value ?
        static_cast<f64>(1) :
        static_cast<f64>(0)
    );
}

// -*-
bool Bool::operator!(){
    return !this->m_value;
}

// -*-
bool operator||(const Bool& lhs, const Bool& rhs){
    return (lhs.as_bool() || rhs.as_bool());
}

// -*-
bool operator&&(const Bool& lhs, const Bool& rhs){
    return (lhs.as_bool() && rhs.as_bool());
}

// --------------------------------------------------------------
// --- Number: wrapper around f64, i64, and std::complex<f64> ---
// --------------------------------------------------------------
const Number Number::PI = Number(3.141592653589793);
const Number Number::E = Number(2.718281828459045);
const Number Number::FMAX = Number(std::numeric_limits<f64>::max());
const Number Number::FMIN = Number(std::numeric_limits<f64>::min());
const Number Number::IMAX = Number(std::numeric_limits<i64>::max());
const Number Number::IMIN = Number(std::numeric_limits<i64>::min());
const Number Number::EPSILON = Number(std::numeric_limits<f64>::epsilon());
const Number Number::PHI = Number(1.618033988749894);
const Number Number::NaN = Number(std::numeric_limits<f64>::quiet_NaN());

// -*-
Number::Number() noexcept
: m_kind{Kind::INT}
, m_value{i64{}}
{}

// -*-
Number::Number(f64 num) noexcept
: Number{}
{
    this->m_kind = Kind::FLOAT;
    this->m_value = num;
}

// -*-
Number::Number(i64 num) noexcept
: Number{}{
    this->m_kind = Kind::INT;
    this->m_value = num;
}

// -*-
Number::Number(f64 x, f64 y) noexcept
: Number{}{
    this->m_kind = Kind::COMPLEX;
    this->m_value = std::complex<f64>(x, y);
}

// -*-
Number::Number(const Complex& z) noexcept
: Number{}{
    this->m_kind = Kind::COMPLEX;
    this->m_value = std::complex<f64>(z.real(), z.imag());
}

Number::Number(const Number& number) noexcept
: m_kind{number.m_kind}
, m_value{number.m_value}
{}

Number::Number(Number&& number) noexcept
: m_kind{std::move(number.m_kind)}
, m_value{std::move(number.m_value)}
{}

// -*-
Number& Number::operator=(const Number& number) noexcept{
    if(this != &number){
        this->m_kind = number.m_kind;
        this->m_value = number.m_value;
    }
    return *this;
}

// -*-
Number& Number::operator=(Number&& number) noexcept{
    if(this != &number){
        this->m_kind = std::move(number.m_kind);
        this->m_value = std::move(number.m_value);
    }
    return *this;
}

bool Number::is_integer(void) const{
    return this->m_kind == Kind::INT;
}

// -*-
bool Number::is_float(void) const{
    return this->m_kind==Kind::FLOAT;
}

// -*-
bool Number::is_complex(void) const{
    return this->m_kind==Kind::COMPLEX;
}

// -*-
Symbol Number::type(void) const{
    if(this->is_integer()){
        return Symbol("integer");
    }else if(this->is_float()){
        return Symbol("float");
    }
    return Symbol("complex");
}

// -*-
Str Number::str(void) const{
    std::stringstream stream;
    if(this->is_integer()){
        i64 num{};
        this->get(num);
        stream << num;
    }else if(this->is_float()){
        f64 num{};
        this->get(num);
        stream << num;
    }else{
        Complex z{};
        this->get(z);
        if(z.real()==0.0){
            stream << z.imag() << "i";
        }else{
            stream << z.real() << " + " << z.imag() << "i";
        }
    }

    return stream.str();
}

// -*-
Str Number::repr(void) const{
    if(this->is_integer() || this->is_float()){
        return this->str();
    }
    std::stringstream stream;
    Complex z{};
    this->get(z);
    auto x = z.real();
    auto y = z.imag();
    stream << "(complex " << x << " " << y << ")";
    return stream.str();
}

bool Number::as_bool(void) const{
    if(this->is_integer()){
        i64 num{};
        this->get(num);
        return num==0;
    }else if(this->is_float()){
        f64 num{};
        this->get(num);
        return (num==0.0);
    }
    return true; // complex numbers always cnvert to true
}

// -*-
i64 Number::as_integer(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "complex number cannot convert to integer");
    }
    if(this->is_integer()){
        i64 num{};
        this->get(num);
        return num;
    }
    f64 num{};
    this->get(num);
    return static_cast<i64>(num);
}

// -*-
f64 Number::as_float(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "complex number cannot convert to float");
    }
    if(this->is_integer()){
        i64 num{};
        this->get(num);
        return static_cast<f64>(num);
    }
    f64 num{};
    this->get(num);
    return num;
}

// -*-
Complex Number::as_complex(void) const{
    if(this->is_integer() || this->is_float()){
        auto x = this->as_float();
        return std::complex<f64>(x, 0.0);
    }
    Complex z{};
    this->get(z);
    return z;
}

// -*-
Number Number::operator-(){
    if(this->is_integer()){
        i64 num{};
        this->get(num);
        return Number(-num);
    }else if(this->is_float()){
        f64 num{};
        this->get(num);
        return Number(-num);
    }
    Complex z{};
    this->get(z);
    return Number(-z);
}

/*
Number operator+(const Number& lhs, const Number& rhs){}
Number operator-(const Number& lhs, const Number& rhs){}
Number operator*(const Number& lhs, const Number& rhs){}
Number operator/(const Number& lhs, const Number& rhs){}
Number operator%(const Number& lhs, const Number& rhs){}

bool operator==(const Number& lhs, const Number& rhs){}
bool operator!=(const Number& lhs, const Number& rhs){}
bool operator<=(const Number& lhs, const Number& rhs){}
bool operator>=(const Number& lhs, const Number& rhs){}
bool operator<(const Number& lhs, const Number& rhs){}
bool operator>(const Number& lhs, const Number& rhs){}

Number Number::abs(void) const{}
Number Number::min(const Number& other) const{}
Number Number::max(const Number& other) const{}
Number Number::floor(void) const{}
Number Number::ceil(void) const{}
Number Number::round(void) const{}
Number Number::log(void) const{}
Number Number::log2(void) const{}
Number Number::log10(void) const{}
Number Number::log1p(void) const{}
Number Number::exp(void) const{}
Number Number::exp2(void) const{}
Number Number::expm1(void) const{}
Number Number::pow(const Number& other) const{}
Number Number::sqrt(void) const{}
Number Number::cbrt(void) const{}
Number Number::hypot(const Number& other) const{}
Number Number::sin(void) const{}
Number Number::cos(void) const{}
Number Number::tan(void) const{}
Number Number::asin(void) const{}
Number Number::acos(void) const{}
Number Number::atan(void) const{}
Number Number::atan2(void) const{}
Number Number::sinh(void) const{}
Number Number::cosh(void) const{}
Number Number::tanh(void) const{}
Number Number::asinh(void) const{}
Number Number::acosh(void) const{}
Number Number::atanh(void) const{}
Number Number::erf(void) const{}
Number Number::erfc(void) const{}
Number Number::tgamma(void) const{}
Number Number::lgamma(void) const{}
Number Number::isfinite(void) const{}
Number Number::isinf(void) const{}
Number Number::isnan(void) const{}
*/

// --------------
// -*- Symbol -*-
// --------------
/*
Symbol::Symbol(const Str& str) noexcept{}
Symbol Symbol::type(void) const{}
Str Symbol::str(void) const{}
Str Symbol::repr(void) const{}

bool operator==(const Symbol& lhs, const Symbol& rhs);
bool operator!=(const Symbol& lhs, const Symbol& rhs);
*/

// --------------
// -*- String -*-
// --------------
/*
String::String() noexcept{}
String::String(const Str& str) noexcept{}

Symbol String::type(void) const{}
Str String::str(void) const{}
Str String::repr(void) const{}

bool operator+(const String& lhs, const String& rhs){}
bool operator==(const String& lhs, const String& rhs){}
bool operator!=(const String& lhs, const String& rhs){}
bool operator<=(const String& lhs, const String& rhs){}
bool operator>=(const String& lhs, const String& rhs){}
bool operator<(const String& lhs, const String& rhs){}
bool operator>(const String& lhs, const String& rhs){}

i64 String::len(void) const{}
String String::capitalize(void) const{}
String String::upper(void) const{}
String String::lower(void) const{}
String String::trim(void) const{}
String String::ltrim(void) const{}
String String::rtrim(void) const{}
String String::join(const Vec<String>& vec) const{}
String String::replace(const String& old, const String& neo) const{}
Vec<String> String::split(const String& delim){}
i64 String::find(const String& needle) const{}
String String::substr(i64 start=0, i64 end=Str::npos) const{}
bool String::contains(const String& needle) const{}
bool String::startswith(const String& prefix) const{}
bool String::endswith(const String& suffix) const{}
*/

// ------------
// -*- List -*-
// ------------
/*
List::List(const Vec<Self>& xs) noexcept{}
List::List(const std::list<Self>& xs) noexcept{}

bool List::is_nil(void) const{}
bool List::is_list(void) const{}
Symbol List::type(void) const{}
Str List::str(void) const{}
Str List::repr(void) const{}

std::list<Self> List::as_list(void) const{}
Vec<Self> List::as_vector(void) const{}

i64 List::len(void) const{}
Self List::head(void) const{}
List List::tail(void) const{}
Self List::last(void) const{}
Self List::nth(i64 idx) const{}
List List::push(const Self& self){}
Self List::pop(void){}
List List::append(const Self& self){}
List List::insert(i64 idx, const Self& self){}
List List::remove(i64 idx){}
List List::set(i64 idx, const Self& self){}
    
List operator+(const List& lhs, const List& rhs){}
*/

// ---------------
// -*- Builtin -*-
// ---------------
/*
Builtin::Builtin(const Str& name, CFun cfun, i32 minArgc, i32 maxArgc) noexcept{}
Builtin::Builtin(const Builtin& builtin) noexcept{}
Builtin::Builtin(Builtin&& builtin) noexcept{}
Builtin& Builtin::operator=(const Builtin& builtin) noexcept{}
Builtin& Builtin::operator=(Builtin&& builtin) noexcept{}

Symbol Builtin::type(void) const{}
Str Builtin::str(void) const{}
Str Builtin::repr(void) const{}

i64 Builtin::min_argc(void) const{}
i64 Builtin::max_argc(void) const{}
const Str& Builtin::name(void) const{}

Result Builtin::operator()(const Vec<Self>& args){}
*/

// ---------------
// --- Closure ---
// ---------------
/*
Closure::Closure(const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept;
Closure::Closure(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept{}
Closure::Closure(const Closure& closure) noexcept{}
Closure::Closure(Closure&& closure) noexcept{}
Closure& Closure::operator=(const Closure& closure) noexcept{}
Closure& Closure::operator=(Closure&& closure) noexcept{}

bool Closure::is_callable(void) const{}
bool Closure::is_lambda(void) const{}
bool Closure::is_function(void) const{}
Symbol Closure::type(void) const{}
Str Closure::str(void) const{}
Str Closure::repr(void) const{}

i64 Closure::argc(void) const{}
Result Closure::operator()(const Vec<Self>& args){}
*/

// -------------
// --- Macro ---
// -------------
/*
class Macro:: final: public Object{
public:
Macro::Macro(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept{}
Macro::Macro(const Macro& macro) noexcept{}
Macro::Macro(Closure&& macro) noexcept{}
Macro& Macro::operator=(const Macro& macro) noexcept{}
Macro& Macro::operator=(Macro&& macro) noexcept{}

Symbol Macro::type(void) const{}
Str Macro::str(void) const{}
Str Macro::repr(void) const{}

i64 Macro::argc(void) const{}
Self Macro::expand(const Vec<Self>& args) const{}
Result operator()(const Vec<Self>& args){}
*/

// -------------------------------------------------------------------------------
// Convenient functions to create shared pointer of Object and its derived classes
// -------------------------------------------------------------------------------
/*
Self share(void){}
Self share(bool val){}
Self share(i64 num){}
Self share(f64 num){}
Self share(f64 x, f64 y){}
Self share(const Complex& z){}
Self share(const char* sym){}
Self share(const Str& str){}
Self share(const Vec<Self>& xs){}
Self share(const std::list<Self>& xs){}
Self share(const Str& name, CFun fun, i64 minArgc, i64 maxArgc){}
Self share(const Vec<Symbol>& params, const Vec<Self>& body, const Env& env){}
Self share(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& env, bool macro){}
*/

// -------------------------------------------------------
// Convenient operators for Object and its derived classes
// -------------------------------------------------------
/*
Self operator+(const Self& lhs, const Self& rhs){}
Self operator-(const Self& lhs, const Self& rhs){}
Self operator*(const Self& lhs, const Self& rhs){}
Self operator/(const Self& lhs, const Self& rhs){}
Self operator%(const Self& lhs, const Self& rhs){}

bool operator==(const Self& lhs, const Self& rhs){}
bool operator!=(const Self& lhs, const Self& rhs){}
bool operator<=(const Self& lhs, const Self& rhs){}
bool operator>=(const Self& lhs, const Self& rhs){}
bool operator<(const Self& lhs, const Self& rhs){}
bool operator>(const Self& lhs, const Self& rhs){}

bool operator||(const Self& lhs, const Self& rhs){}
bool operator&&(const Self& lhs, const Self& rhs){}
*/

// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-

