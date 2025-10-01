#include "lynx.hpp"

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

//-*-
Number operator+(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto n1 = lhs.as_integer();
        auto n2 = rhs.as_integer();
        return Number((n1+n2));
    }else if(!lhs.is_complex() && !rhs.is_complex()){
        auto x = lhs.as_float();
        auto y = rhs.as_float();
        return Number((x+y));
    }else if(lhs.is_complex() && (rhs.is_integer() || rhs.is_float())){
        auto z = lhs.as_complex();
        auto x = rhs.as_float();
        return Number(z+x); 
    }else if((lhs.is_integer() || lhs.is_float()) && rhs.is_complex()){
        auto x = lhs.as_float();
        auto z = rhs.as_complex();
        return Number(z+x);
    }
    auto z1 = lhs.as_complex();
    auto z2 = rhs.as_complex();
    return Number((z1+z2));
}


Number operator-(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto n1 = lhs.as_integer();
        auto n2 = rhs.as_integer();
        return Number((n1-n2));
    }else if(!lhs.is_complex() && !rhs.is_complex()){
        auto x = lhs.as_float();
        auto y = rhs.as_float();
        return Number((x-y));
    }else if(lhs.is_complex() && (rhs.is_integer() || rhs.is_float())){
        auto z = lhs.as_complex();
        auto x = rhs.as_float();
        return Number(z-x); 
    }else if((lhs.is_integer() || lhs.is_float()) && rhs.is_complex()){
        auto x = lhs.as_float();
        auto z = rhs.as_complex();
        return Number(x-z);
    }
    auto z1 = lhs.as_complex();
    auto z2 = rhs.as_complex();
    return Number((z1-z2));
}

// -*-
Number operator*(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto n1 = lhs.as_integer();
        auto n2 = rhs.as_integer();
        return Number((n1*n2));
    }else if(!lhs.is_complex() && !rhs.is_complex()){
        auto x = lhs.as_float();
        auto y = rhs.as_float();
        return Number((x*y));
    }else if(lhs.is_complex() && (rhs.is_integer() || rhs.is_float())){
        auto z = lhs.as_complex();
        auto x = rhs.as_float();
        return Number(z*x); 
    }else if((lhs.is_integer() || lhs.is_float()) && rhs.is_complex()){
        auto x = lhs.as_float();
        auto z = rhs.as_complex();
        return Number(x*z);
    }
    auto z1 = lhs.as_complex();
    auto z2 = rhs.as_complex();
    return Number((z1*z2));
}

// -*-
Number operator/(const Number& lhs, const Number& rhs){
    if(lhs.is_integer() && rhs.is_integer()){
        auto n1 = lhs.as_integer();
        auto n2 = rhs.as_integer();
        if(n2==0){
            throw Error(Error::Kind::ValueError, "division by zero");
        }
        return Number((n1/n2));
    }else if(!lhs.is_complex() && !rhs.is_complex()){
        auto x = lhs.as_float();
        auto y = rhs.as_float();
        if(y==0.0){
            throw Error(Error::Kind::ValueError, "division by zero");
        }
        return Number((x/y));
    }else if(lhs.is_complex() && (rhs.is_integer() || rhs.is_float())){
        auto z = lhs.as_complex();
        auto x = rhs.as_float();
        if(x==0.0){
            throw Error(Error::Kind::ValueError, "division by zero");
        }
        return Number(z/x);
    }else if((lhs.is_integer() || lhs.is_float()) && rhs.is_complex()){
        auto x = lhs.as_float();
        auto z = rhs.as_complex();
        if(std::norm(z)==0.0){
            throw Error(Error::Kind::ValueError, "division by zero");
        }
        return Number(x/z);
    }
    auto z1 = lhs.as_complex();
    auto z2 = rhs.as_complex();
    if(std::norm(z2)==0.0){
        throw Error(Error::Kind::ValueError, "division by zero");
    }
    return Number((z1/z2));
}

// -*-
Number operator%(const Number& lhs, const Number& rhs){
    if(lhs.is_complex() || rhs.is_complex()){
        throw Error(Error::Kind::TypeError, "`%` is not supported for complex numbers.");
    }
    if(lhs.is_integer() && rhs.is_integer()){
        auto x = lhs.as_integer();
        auto y = rhs.as_integer();
        return Number((x % y));
    }
    auto x = lhs.as_float();
    auto y = rhs.as_float();
    return Number(std::fmod(x, y));
}

// -*-
bool operator==(const Number& lhs, const Number& rhs){
    if(lhs.is_complex() && rhs.is_complex()){
        auto z1 = lhs.as_complex();
        auto z2 = rhs.as_complex();
        return (
            (z1.real()==z2.real()) &&
            (z1.imag()==z2.imag())
        );
    }else if(lhs.is_integer() && rhs.is_integer()){
        return (lhs.as_integer()==rhs.as_integer());
    }else if(!lhs.is_complex() && !rhs.is_complex()){
        auto x = lhs.as_float();
        auto y = rhs.as_float();
        return x==y;
    }
    return false;
}

// -*-
bool operator!=(const Number& lhs, const Number& rhs){
    return !(lhs==rhs);
}

bool operator<=(const Number& lhs, const Number& rhs){
    return (lhs< rhs) || (lhs == rhs);
}

// -*-
bool operator>=(const Number& lhs, const Number& rhs){
    return (lhs > rhs) || (lhs == rhs);
}

// -*-
bool operator<(const Number& lhs, const Number& rhs){
    if(lhs.is_complex() && rhs.is_complex()){
        auto z1 = lhs.as_complex();
        auto z2 = rhs.as_complex();
        return (
            (z1.real() < z2.real()) &&
            (z1.imag() < z2.imag())
        );
    }else if(lhs.is_integer() && rhs.is_integer()){
        return (lhs.as_integer() < rhs.as_integer());
    }else if((!lhs.is_complex() && rhs.is_complex()) || (lhs.is_complex() && !rhs.is_complex())){
        auto _ty1 = lhs.type().str();
        auto _ty2 = rhs.type().str();
        std::stringstream ss;
        ss << "`<' is not applicable between `" << _ty1 << "' and `" << _ty2 << "' objects";
        throw Error(Error::Kind::TypeError, ss.str());
    }
    return (lhs.as_float() < rhs.as_float());
}

// -*-
bool operator>(const Number& lhs, const Number& rhs){
    return !(lhs <= rhs);
}

// -*-
Number Number::real(void) const{
    if(this->is_complex()){
        Complex z{};
        this->get(z);
        return Number(z.real());
    }
    return Number(this->as_float());
}

// -*-
Number Number::imag(void) const{
    if(this->is_complex()){
        Complex z{};
        this->get(z);
        return Number(z.imag());
    }
    return Number(this->as_float());
}

// -*-
Number Number::arg(void) const{
    auto z = this->as_complex();
    return Number(std::arg(z));
}

// -*-
Number Number::norm(void) const{
    return Number(std::norm(this->as_complex()));
}

// -*-
Number Number::conj(void) const{
    return Number(std::conj(this->as_complex()));
}

// -*-
Number Number::polar(f64 rho, f64 theta) const{
    return Number(std::polar(rho, theta));
}

Number Number::abs(void) const{
    if(this->is_complex()){
        return Number(std::abs(this->as_complex()));
    }else if(this->is_float()){
        return Number(std::abs(this->as_float()));
    }
    return Number(std::abs(this->as_integer()));
}

// -*-
Number Number::min(const Number& other) const{
    if(this->is_complex() || other.is_complex()){
        auto _ty1 = this->type().str();
        auto _ty2 = other.type().str();
        std::stringstream ss;
        ss << "`min' cannot be applied between `" << _ty1 << "' and `";
        ss << _ty2 << "' objects";
        throw Error(Error::Kind::TypeError, ss.str());
    }else if(this->is_integer() || other.is_integer()){
        auto x = this->as_integer();
        auto y = other.as_integer();
        return Number(std::min(x, y));
    }
    auto x = this->as_float();
    auto y = other.as_float();
    return Number(std::min(x, y));
}

// -*-
Number Number::max(const Number& other) const{
    if(this->is_complex() || other.is_complex()){
        auto _ty1 = this->type().str();
        auto _ty2 = other.type().str();
        std::stringstream ss;
        ss << "`max' cannot be applied between `" << _ty1 << "' and `";
        ss << _ty2 << "' objects";
        throw Error(Error::Kind::TypeError, ss.str());
    }else if(this->is_integer() || other.is_integer()){
        auto x = this->as_integer();
        auto y = other.as_integer();
        return Number(std::max(x, y));
    }
    auto x = this->as_float();
    auto y = other.as_float();
    return Number(std::max(x, y));
}

// -*-
Number Number::floor(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`floor` is not supported for complex numbers");
    }
    return Number(std::floor(this->as_float()));
}

// -*-
Number Number::ceil(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`ceil` is not supported for complex numbers");
    }
    return Number(std::ceil(this->as_float()));
}

// -*-
Number Number::round(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`round` is not supported for complex numbers");
    }
    return Number(std::round(this->as_float()));
}

// -*-
Number Number::log(void) const{
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    if(this->is_complex()){
        [[maybe_unused]] auto z = std::log(this->as_complex());
        if(errno!=0){
            Str msg(std::strerror(errno));
            throw Error(Error::Kind::ValueError, msg);
        }
        return Number(z);
    }
    [[maybe_unused]] auto x = std::log(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::log2(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`log2' is not implemented for complex numbers");
    }
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    [[maybe_unused]] auto x = std::log2(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::log10(void) const{
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    if(this->is_complex()){
        [[maybe_unused]] auto z = std::log10(this->as_complex());
        if(errno!=0){
            Str msg(std::strerror(errno));
            throw Error(Error::Kind::ValueError, msg);
        }
        return Number(z);
    }
    [[maybe_unused]] auto x = std::log10(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::log1p(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`log1p' is not implemented for complex numbers");
    }
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    [[maybe_unused]] auto x = std::log1p(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::exp(void) const{
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    if(this->is_complex()){
        [[maybe_unused]] auto z = std::exp(this->as_complex());
        if(errno!=0){
            Str msg(std::strerror(errno));
            throw Error(Error::Kind::ValueError, msg);
        }
        return Number(z);
    }
    [[maybe_unused]] auto x = std::exp(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::exp2(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`exp2' is not implemented for complex numbers");
    }
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    [[maybe_unused]] auto x = std::exp2(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::expm1(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`expm1' is not implemented for complex numbers");
    }
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    [[maybe_unused]] auto x = std::expm1(this->as_float());
    if(errno!=0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }

    return Number(x);
}

// -*-
Number Number::pow(const Number& rhs) const{
    auto test = (
        (!this->is_complex() && rhs.is_complex()) ||
        (this->is_complex() && !rhs.is_complex()) ||
        (this->is_complex() && rhs.is_complex())
    );
    if(test){
        auto z1 = this->as_complex();
        auto z2 = rhs.as_complex();
        return Number(std::pow(z1, z2));
    }
    auto x = this->as_float();
    auto y = rhs.as_float();
    return Number(std::pow(x, y));
}

// -*-
Number Number::sqrt(void) const{
    if(this->is_complex()){
        auto z = this->as_complex();
        return Number(std::sqrt(z));
    }
    auto x = this->as_float();
    if(x < 0){
        throw Error(Error::Kind::ValueError, "square of negative number.");
    }
    return Number(x);
}

// -*-
Number Number::cbrt(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`cbrt` not supported for complex numbers");
    }
    auto x = this->as_float();
    errno = 0;
    std::feclearexcept(FE_ALL_EXCEPT);
    auto ans = std::cbrt(x);
    if(errno != 0){
        Str msg(std::strerror(errno));
        throw Error(Error::Kind::ValueError, msg);
    }
    return Number(ans);
}

// -*-
Number Number::hypot(const Number& rhs) const{
    if(this->is_complex() || rhs.is_complex()){
        throw Error(Error::Kind::TypeError, "`hypot` is not supported for complex numbers.");
    }
    auto x = this->as_float();
    auto y = rhs.as_float();
    return Number(std::hypot(x, y));
}

// -*-
Number Number::sin(void) const{
    if(this->is_complex()){
        return Number(std::sin(this->as_complex()));
    }
    return Number(std::sin(this->as_float()));
}

// -*-
Number Number::cos(void) const{
    if(this->is_complex()){
        return Number(std::cos(this->as_complex()));
    }
    return Number(std::cos(this->as_float()));
}

// -*-
Number Number::tan(void) const{
    if(this->is_complex()){
        return Number(std::tan(this->as_complex()));
    }
    return Number(std::tan(this->as_float()));
}

// -*-
Number Number::asin(void) const{
    if(this->is_complex()){
        return Number(std::asin(this->as_complex()));
    }
    return Number(std::asin(this->as_float()));
}

// -*-
Number Number::acos(void) const{
    if(this->is_complex()){
        return Number(std::acos(this->as_complex()));
    }
    return Number(std::acos(this->as_float()));
}

// -*-
Number Number::atan(void) const{
    if(this->is_complex()){
        return Number(std::atan(this->as_complex()));
    }
    return Number(std::atan(this->as_float()));
}

// -*-
Number Number::atan2(const Number& rhs) const{
    if(this->is_complex() || rhs.is_complex()){
        throw Error(Error::Kind::TypeError, "`atan2` is not supported for complex numbers.");
    }
    auto y = this->as_float();
    auto x = rhs.as_float();
    return Number(std::atan2(y, x));
}

// -*-
Number Number::sinh(void) const{
    if(this->is_complex()){
        return Number(std::sinh(this->as_complex()));
    }
    return Number(std::sinh(this->as_float()));
}

// -*-
Number Number::cosh(void) const{
    if(this->is_complex()){
        return Number(std::cosh(this->as_complex()));
    }
    return Number(std::cos(this->as_float()));
}

Number Number::tanh(void) const{
    if(this->is_complex()){
        return Number(std::tanh(this->as_complex()));
    }
    return Number(std::tanh(this->as_float()));
}

// -*-
Number Number::asinh(void) const{
    if(this->is_complex()){
        return Number(std::asinh(this->as_complex()));
    }
    return Number(std::asinh(this->as_float()));
}

// -*-
Number Number::acosh(void) const{
    if(this->is_complex()){
        return Number(std::acosh(this->as_complex()));
    }
    return Number(std::acosh(this->as_float()));
}

// -*-
Number Number::atanh(void) const{
    if(this->is_complex()){
        return Number(std::atanh(this->as_complex()));
    }
    return Number(std::atanh(this->as_float()));
}

// -*-
Number Number::erf(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`erf` is not supported for complex numbers");
    }
    return Number(std::erf(this->as_float()));
}

// -*-
Number Number::erfc(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`erfc` is not supported for complex numbers");
    }
    return Number(std::erfc(this->as_float()));
}

// -*-
Number Number::tgamma(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`tgamma` is not supported for complex numbers");
    }
    return Number(std::tgamma(this->as_float()));
}

// -*-
Number Number::lgamma(void) const{
    if(this->is_complex()){
        throw Error(Error::Kind::TypeError, "`lgamma` is not supported for complex numbers");
    }
    return Number(std::lgamma(this->as_float()));
}

// -*-
bool Number::isfinite(void) const{
    if(this->is_complex()){
        auto z = this->as_complex();
        return (std::isfinite(z.real()) && std::isfinite(z.imag()));
    }
    return std::isfinite(this->as_float());
}

// -*-
bool Number::isinf(void) const{
    if(this->is_complex()){
        auto z = this->as_complex();
        return (std::isinf(z.real()) || std::isinf(z.imag()));
    }
    return std::isinf(this->as_float());
}

// -*-
bool Number::isnan(void) const{
    if(this->is_complex()){
        auto z = this->as_complex();
        return (std::isnan(z.real()) || std::isnan(z.imag()));
    }
    return std::isinf(this->as_float());
}

// --------------
// -*- Symbol -*-
// --------------
Symbol::Symbol(const Str& str) noexcept
: m_str{str}
{}

// -*-
Symbol Symbol::type(void) const{
    return Symbol("symbol");
}

// -*-
Str Symbol::str(void) const{
    return this->m_str;
}

Str Symbol::repr(void) const{
    return this->m_str;
}

// -*-
bool operator==(const Symbol& lhs, const Symbol& rhs){
    return (lhs.str() == rhs.str());
}

// -*-
bool operator!=(const Symbol& lhs, const Symbol& rhs){
    return !(lhs == rhs);
}

// --------------
// -*- String -*-
// --------------
String::String() noexcept
: m_str{}
{}

// -*-
String::String(const Str& str) noexcept
: m_str{str}
{}

// -*-
String::String(const String& other) noexcept
: m_str{other.m_str}
{}

String::String(String&& other) noexcept
: m_str{std::move(other.m_str)}
{}

// -*-
String& String::operator=(const String& other) noexcept{
    if(this != &other){
        this->m_str = other.m_str;
    }
    return *this;
}

// -*-
String& String::operator=(String&& other) noexcept{
    if(this != &other){
        this->m_str = std::move(other.m_str);
    }
    return *this;
}

// -*-
Symbol String::type(void) const{
    return Symbol("string");
}

// -*-
Str String::str(void) const{
    return this->m_str;
}

// -*-
Str String::repr(void) const{
    std::stringstream ss;
    ss << std::quoted(this->m_str);
    return ss.str();
}

// -*-
String operator+(const String& lhs, const String& rhs){
    auto str = (lhs.str() + rhs.str());
    return String(str);
}

// -*-
bool operator==(const String& lhs, const String& rhs){
    return (lhs.str() == rhs.str());
}

// -*-
bool operator!=(const String& lhs, const String& rhs){
    return !(lhs==rhs);
}

// -*-
bool operator<=(const String& lhs, const String& rhs){
    return ((lhs < rhs) || (lhs == rhs));
}

// -*-
bool operator>=(const String& lhs, const String& rhs){
    return (lhs > rhs) || (lhs == rhs);
}

// -*-
bool operator<(const String& lhs, const String& rhs){
    return (lhs.str() < rhs.str());
}

// -*-
bool operator>(const String& lhs, const String& rhs){
    return !(lhs >= rhs);
}

// -*-
i64 String::len(void) const{
    return static_cast<i64>(this->str().length());
}

// -*-
String String::capitalize(void) const{
    auto ans = this->str();
    bool first = false;
    size_t idx = 0;
    for(auto i=0; i < ans.length(); i++){
        if(std::isalpha(ans[i])){
            first = true;
            idx = i;
        }
        if(first){ break; }
    }
    ans[idx] = std::toupper(ans[idx]);
    return String(ans);
}

// -*-
String String::upper(void) const{
    auto ans = this->str();
    std::transform(
        ans.cbegin(), ans.cend(), ans.begin(),
        [](int c){ return std::toupper(c); }
    );

    return String(ans);
}

// -*-
String String::lower(void) const{
    auto ans = this->str();
    std::transform(
        ans.cbegin(), ans.cend(), ans.begin(),
        [](int c){ return std::tolower(c); }
    );

    return String(ans);
}

// -*-
String String::ltrim(void) const{
    auto ans = this->str();
    ans.erase(
        ans.begin(),
        std::find_if(
            ans.begin(), ans.end(),
            [](int c) {return !std::isspace(c);}
        )
    );
    return String(ans);
}

// -*-
String String::rtrim(void) const{
    auto ans = this->str();
    ans.erase(std::find_if(
            ans.rbegin(), ans.rend(),
            [](int ch) { return !std::isspace(ch); }
        ).base(),
        ans.end(
    ));

    return String(ans);
}

// -*-
String String::trim(void) const{
    return this->ltrim().rtrim();
}

// -*-
String String::join(const Vec<String>& vec) const{
    if(vec.size()==0){
        return String();
    }
    if(vec.size()==1){
        return vec[0];
    }
    std::stringstream ss;
    auto sep = this->str();
    ss << vec[0].str();
    for(size_t i=1; i < vec.size(); i++){
        ss << sep << vec[i].str();
    }

    return String(ss.str());
}

// -*-
String String::replace(const String& old, const String& neo) const{
    auto text = this->str();
    auto _old = old.str();
    auto _new = neo.str();
    auto pos = text.find(_old);
    if(pos!=Str::npos){
        text.replace(pos, _old.length(), _new);
    }

    return String(text);
}

// -*-
Vec<String> String::split(const String& delim){
    Vec<String> result{};
    auto sep = delim.str();
    auto text = this->str();
    String tmp{};
    auto pos = text.find(sep);
    while(pos!=Str::npos){
        tmp = String(text.substr(pos));
        result.push_back(tmp.trim());
        int n = static_cast<int>(tmp.len() + delim.len());
        auto first = text.begin();
        auto last = first + n;
        text.erase(first, last);
        pos = text.find(sep);
    }

    if(text.length() != 0){ result.push_back(String(text).trim());}
    return result;
}

// -*-
i64 String::find(const String& needle) const{
    auto text = this->str();
    auto key = needle.str();
    auto pos = text.find(key);
    return (pos==Str::npos) ? -1 : static_cast<i64>(pos);
}

// -*-
String String::substr(i64 start, i64 end) const{
    auto text = this->str();
    auto ans = text.substr(start, end);
    return String(ans);
}

// -*-
bool String::contains(const String& needle) const{
    auto text = this->str();
    auto key = needle.str();
    auto pos = text.find(key);
    return (pos==Str::npos) ? false : true;
}

// -*-
bool String::startswith(const String& prefix) const{
    if(prefix.len() > this->len()){ return false; }
    auto text = this->str();
    auto key = prefix.str();
    return (key == text.substr(key.length()));
}

// -*-
bool String::endswith(const String& suffix) const{
    if(this->len() < suffix.len()){ return false; }
    auto start = this->len() - suffix.len();
    return (this->str().substr(start)==suffix.str());
}


// ------------
// -*- List -*-
// ------------
List::List(const Vec<Self>& xs) noexcept
: m_value{std::list<Self>(xs.cbegin(), xs.cend())}
{}

List::List(const std::list<Self>& xs) noexcept
: m_value{std::list<Self>(xs.cbegin(), xs.cend())}
{}

// -*-
bool List::is_nil(void) const{
    return (this->m_value.size()==0);
}

// -*-
bool List::is_list(void) const{
    return true;
}

// -*-
Symbol List::type(void) const{
    return Symbol("list");
}

// -*-
Str List::str(void) const{
    auto const N = this->m_value.size();
    if(N==0){ return "nil"; }
    if(N==1){
        Str result{"("};
        result += this->m_value.back()->str();
        result += ")";
        return result;
    }
    std::stringstream ss;
    int idx = 0;
    auto start = this->m_value.cbegin();
    ss << "(";
    auto ptr = std::next(start, idx);
    ss << (*ptr)->str();

    while(++idx < N){
        ptr = std::next(start, idx);
        ss << " " << (*ptr)->str();
    }
    ss << ")";
    return ss.str();
}

// -*-
Str List::repr(void) const{
    auto const N = this->m_value.size();
    if(N==0){ return "nil"; }
    if(N==1){
        Str result{"("};
        result += this->m_value.back()->repr();
        result += ")";
        return result;
    }
    std::stringstream ss;
    int idx = 0;
    auto start = this->m_value.cbegin();
    ss << "(";
    auto ptr = std::next(start, idx);
    ss << (*ptr)->repr();

    while(++idx < N){
        ptr = std::next(start, idx);
        ss << " " << (*ptr)->repr();
    }
    ss << ")";
    return ss.str();
}

// -*-
std::list<Self> List::as_list(void) const{
    return this->m_value;
}

// -*-
Vec<Self> List::as_vector(void) const{
    return Vec<Self>(this->m_value.cbegin(), this->m_value.cend());
}

// -*-
i64 List::len(void) const{
    return static_cast<i64>(this->m_value.size());
}

// -*-
Self List::head(void) const{
    if(this->len()==0){
        throw Error(Error::Kind::ValueError, "cannot get `head' of an empty list");
    }
    auto self = this->m_value.front();
    return self;
}

// -*-
List List::tail(void) const{
    if(this->len()==0){
        return List(Vec<Self>());
    }

    auto ptr = std::next(this->m_value.cbegin(), 1);
    auto end = this->m_value.cend();
    return List(Vec<Self>(ptr, end));
}

Self List::last(void) const{
    if(this->len()==0){
        throw Error(Error::Kind::ValueError, "cannot get `last' of an empty list");
    }
    auto self = this->m_value.back();
    return self;
}

// -*-
Self List::nth(i64 idx) const{
    if(idx < 0 || idx >= this->len()){
        throw Error(Error::Kind::ValueError, "`nth': index out of range");
    }
    auto ptr = std::next(this->m_value.cbegin(), idx);
    return (*ptr);
}

// -*-
/*
Pre: xs = (1, 2, 3)
(pust xs 6)
Post: xs = (6, 1, 2, 3)
*/
List List::push(const Self& self){
    auto xs = this->as_list();
    xs.insert(xs.begin(), self);
    return List(xs);
}

// -*-
Self List::pop(void){
    if(this->len()==0){
        throw Error(Error::Kind::ValueError, "cannot `pop' an empty list.");
    }
    auto self = this->m_value.back();
    this->m_value.pop_back();
    return self;
}

// -*-
/*
Pre: xs = (1, 2, 3)
(append xs 6)
Post: xs = (1, 2, 3, 6)
*/
List List::append(const Self& self){
    auto xs = this->as_list();
    xs.push_back(self);
    return List(xs);
}

// -*-
List List::insert(i64 idx, const Self& self){
    if(idx < 0 || idx > this->len()){
        throw Error(Error::Kind::ValueError, "`insert': index out of range");
    }
    auto xs = this->as_list();
    auto ptr = xs.begin();
    ptr = std::next(ptr, static_cast<int>(idx));
    xs.insert(ptr, self);
    return List(xs);
}

// -*-
List List::remove(i64 idx){
    if(this->len()){
        throw Error(Error::Kind::ValueError, "`remove': cannot remove from an empty list");
    }
    if(idx < 0 || idx >= this->len()){
        throw Error(Error::Kind::ValueError, "`remove': index out of range");
    }
    auto xs = this->as_list();
    auto ptr = xs.begin();
    auto i = static_cast<int>(idx);
    ptr = std::next(ptr, i);
    xs.erase(ptr);
    return List(xs);
}

// -*-
List List::set(i64 idx, const Self& self){
    if(idx < 0 || idx >= this->len()){
        throw Error(Error::Kind::ValueError, "`remove': index out of range");
    }
    auto xs = this->as_list();
    auto ptr = xs.begin();
    auto i = static_cast<int>(idx);
    ptr = std::next(ptr, i);
    *ptr = self;
    return List(xs);
}

// -*-
List operator+(const List& lhs, const List& rhs){
    auto vec = lhs.as_vector();
    auto xs = rhs.as_vector();
    for(const auto x: xs){
        vec.push_back(x);
    }
    return List(vec);
}

// ---------------
// -*- Builtin -*-
// ---------------
Builtin::Builtin(const Str& name, CFun cfun, i32 minArgc, i32 maxArgc) noexcept
: m_name{name}
, m_cfun{cfun}
, m_minArgc{minArgc}
, m_maxArgc{maxArgc}
{}

Builtin::Builtin(const Builtin& builtin) noexcept
: m_name{builtin.m_name}
, m_cfun{builtin.m_cfun}
, m_minArgc{builtin.m_minArgc}
, m_maxArgc{builtin.m_maxArgc}
{}

// -*-
Builtin::Builtin(Builtin&& builtin) noexcept
: m_name{std::move(builtin.m_name)}
, m_cfun{std::move(builtin.m_cfun)}
, m_minArgc{std::move(builtin.m_minArgc)}
, m_maxArgc{std::move(builtin.m_maxArgc)}
{}

// -*-
Builtin& Builtin::operator=(const Builtin& builtin) noexcept{
    if(this != &builtin){
        this->m_name = builtin.m_name;
        this->m_cfun = builtin.m_cfun;
        this->m_minArgc = builtin.m_minArgc;
        this->m_maxArgc = builtin.m_maxArgc;
    }
    return *this;
}

// -*-
Builtin& Builtin::operator=(Builtin&& builtin) noexcept{
    if(this != &builtin){
        this->m_name = std::move(builtin.m_name);
        this->m_cfun = std::move(builtin.m_cfun);
        this->m_minArgc = std::move(builtin.m_minArgc);
        this->m_maxArgc = std::move(builtin.m_maxArgc);
    }
    return *this;
}

// -*-
Symbol Builtin::type(void) const{
    return Symbol("builtin");
}

// -*-
Str Builtin::str(void) const{
    std::stringstream ss;
    ss << "<builtin::" << this->name() << " at 0x";
    ss << std::hex << this->m_cfun << ">";
    return ss.str();
}

// -*-
Str Builtin::repr(void) const{
    std::stringstream ss;
    ss << "<builtin::" << this->name() << " at 0x";
    ss << std::hex << this->m_cfun << ">";
    return ss.str();
}

// -*-
i64 Builtin::min_argc(void) const{
    return this->m_minArgc;
}

// -*-
i64 Builtin::max_argc(void) const{
    return this->m_maxArgc;
}

// -*-
const Str& Builtin::name(void) const{
    return this->m_name;
}

// -*-
Result Builtin::operator()(const Vec<Self>& args){
    auto argc = args.size();
    if(this->min_argc() != -1 && this->min_argc() > argc){
        std::stringstream ss;
        ss << "`" << this->name() << "': not enough arguments. Expect at least ";
        ss << this->min_argc() << " got " << argc;
        auto err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    if(this->max_argc() != -1 && this->max_argc() > argc){
        std::stringstream ss;
        ss << "`" << this->name() << "': too many arguments. Expect at most ";
        ss << this->min_argc() << " got " << argc;
        auto err = Error(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }

    return this->m_cfun(args);
}

// ---------------
// --- Closure ---
// ---------------
Closure::Closure(const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept
: m_kind{Kind::LAMBDA}
, m_name{}
, m_params{params}
, m_body{body}
, m_env{Env(captures)}
{}

// -*-
Closure::Closure(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept
: m_kind{Kind::FUNC}
, m_name{name}
, m_params{params}
, m_body{body}
, m_env{Env(captures)}
{}

// -*-
Closure::Closure(const Closure& closure) noexcept
: m_kind{closure.m_kind}
, m_name{closure.m_name}
, m_params{closure.m_params}
, m_body{closure.m_body}
, m_env{closure.m_env}
{}

// -*-
Closure::Closure(Closure&& closure) noexcept
: m_kind{std::move(closure.m_kind)}
, m_name{std::move(closure.m_name)}
, m_params{std::move(closure.m_params)}
, m_body{std::move(closure.m_body)}
, m_env{std::move(closure.m_env)}
{}

// -*-
Closure& Closure::operator=(const Closure& closure) noexcept{
    if(this != &closure){
        this->m_kind = closure.m_kind;
        this->m_name = closure.m_name;
        this->m_params = closure.m_params;
        this->m_body = closure.m_body;
        this->m_env = closure.m_env;
    }

    return *this;
}

// -*-
Closure& Closure::operator=(Closure&& closure) noexcept{
    if(this != &closure){
        this->m_kind = std::move(closure.m_kind);
        this->m_name = std::move(closure.m_name);
        this->m_params = std::move(closure.m_params);
        this->m_body = std::move(closure.m_body);
        this->m_env = std::move(closure.m_env);
    }

    return *this;
}

// -*-
Symbol Closure::type(void) const{
    Str ty{};
    ty = this->is_lambda() ? "lambda" : "function";
    return Symbol(ty);
}

// -*-
Str Closure::str(void) const{
    std::stringstream ss;
    if(this->is_lambda()){
        ss << "(lambda ";
        Vec<Self> vec{};
        if(this->m_params.size()==0){
            ss << "()";
        }else{
            for(const auto& p: this->m_params){
                auto sym = p.str();
                auto cstr = sym.c_str();
                vec.push_back(share(cstr));
            }
        }
        ss << List(vec).str() << "\n";
        ss << List(this->m_body).str() << ")";
    }
    if(this->is_function()){
        ss << "(fun " << this->m_name;
        Vec<Self> vec{};
        if(this->m_params.size()==0){
            ss << "()";
        }else{
            for(const auto& p: this->m_params){
                auto sym = p.str();
                auto cstr = sym.c_str();
                vec.push_back(share(cstr));
            }
        }
        ss << List(vec).str() << "\n";
        ss << List(this->m_body).str() << ")";
    }

    return ss.str();
}

// -*-
Str Closure::repr(void) const{
    std::stringstream ss;
    if(this->is_lambda()){
        ss << "(lambda ";
        Vec<Self> vec{};
        if(this->m_params.size()==0){
            ss << "()";
        }else{
            for(const auto& p: this->m_params){
                auto sym = p.repr();
                auto cstr = sym.c_str();
                vec.push_back(share(cstr));
            }
        }
        ss << List(vec).repr() << "\n";
        ss << List(this->m_body).repr() << ")";
    }
    if(this->is_function()){
        ss << "(fun " << this->m_name;
        Vec<Self> vec{};
        if(this->m_params.size()==0){
            ss << "()";
        }else{
            for(const auto& p: this->m_params){
                auto sym = p.repr();
                auto cstr = sym.c_str();
                vec.push_back(share(cstr));
            }
        }
        ss << List(vec).repr() << "\n";
        ss << List(this->m_body).repr() << ")";
    }

    return ss.str();
}

// -*-
i64 Closure::argc(void) const{
    return static_cast<i64>(this->m_params.size());
}

// -*-
Result Closure::operator()(const Vec<Self>& args){
    if(this->argc() != args.size()){
        std::stringstream ss;
        ss << "incorrect number of arguments. Expect ";
        ss << this->argc() << ", got " << args.size();
        Error err(Error::Kind::SyntaxError, ss.str());
        return Result(std::move(err));
    }
    auto ctx = this->m_env;
    // assumes each element in `args' has been already evaluated.
    for(auto i=0; i < args.size(); i++){
        auto key = this->m_params[i].str();
        auto val = args[i];
    }
    auto self = share(this->m_body);
    return Lynx::eval(self, ctx);
}

// -------------
// --- Macro ---
// -------------
Macro::Macro(const Str& name, const Vec<Symbol>& params, const Vec<Self>& body, const Env& captures) noexcept
: m_name{name}
, m_params{params}
, m_body{body}
, m_env{captures}
{}

// -*-
Macro::Macro(const Macro& macro) noexcept
: m_name{macro.m_name}
, m_params{macro.m_params}
, m_body{macro.m_body}
, m_env{macro.m_env}
{}

// -*-
Macro::Macro(Macro&& macro) noexcept
: m_name{std::move(macro.m_name)}
, m_params{std::move(macro.m_params)}
, m_body{std::move(macro.m_body)}
, m_env{std::move(macro.m_env)}
{}

// -*-
Macro& Macro::operator=(const Macro& macro) noexcept{
    if(this != &macro){
        this->m_name = macro.m_name;
        this->m_params = macro.m_params;
        this->m_body = macro.m_body;
        this->m_env = macro.m_env;
    }
    return *this;
}

Macro& Macro::operator=(Macro&& macro) noexcept{
    if(this != &macro){
        this->m_name = std::move(macro.m_name);
        this->m_params = std::move(macro.m_params);
        this->m_body = std::move(macro.m_body);
        this->m_env = std::move(macro.m_env);
    }
    return *this;
}

// -*-
Symbol Macro::type(void) const{
    return Symbol("macro");
}

// -*-
Str Macro::str(void) const{
    std::stringstream ss;
    ss << "(macro " << this->m_name;
        Vec<Self> vec{};
    if(this->m_params.size()==0){
        ss << "()";
    }else{
        for(const auto& p: this->m_params){
            auto sym = p.str();
            auto cstr = sym.c_str();
            vec.push_back(share(cstr));
        }
    }
    ss << List(vec).str() << "\n";
    ss << List(this->m_body).str() << ")";

    return ss.str();
}

// -*-
Str Macro::repr(void) const{
    std::stringstream ss;
    ss << "(macro " << this->m_name;
        Vec<Self> vec{};
    if(this->m_params.size()==0){
        ss << "()";
    }else{
        for(const auto& p: this->m_params){
            auto sym = p.repr();
            auto cstr = sym.c_str();
            vec.push_back(share(cstr));
        }
    }
    ss << List(vec).repr() << "\n";
    ss << List(this->m_body).repr() << ")";

    return ss.str();
}

/*
i64 Macro::argc(void) const{}
Result Macro::expand(const Vec<Self>& args) const{}
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

