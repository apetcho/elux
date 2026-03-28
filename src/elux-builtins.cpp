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
#include<iostream>
#include<iomanip>
#include<sstream>
#include<limits>
#include<cmath>

// -*----------------------------------------------------------------*-
// -*- begin::namespace::klx                                        -*-
// -*----------------------------------------------------------------*-
namespace ekasoft::klx{
// -

Self add(const Vec<Self>& args) {
    if(args.empty()){ return std::make_shared<Number>();}
    bool anyFloat = false;
    for(auto& a : args){
        if(ELux::is_float(a)){
            anyFloat = true;
            break;
        }
    }
    if(anyFloat){
        f64 sum = 0.0;
        for(auto& a : args){ sum += ELux::as_float(a);}
        return std::make_shared<Number>(sum);
    }else{
        i64 sum = 0;
        for(auto& a : args){ sum += ELux::as_float(a); }
        return std::make_shared<Number>(sum);
    }
}

// -*-
Self sub(const Vec<Self>& args){
    if(args.empty()){
        throw std::runtime_error("`-' needs at least one arg");
    }
    bool anyFloat = false;
    for(auto& a : args){
        if(ELux::is_float(a)){
            anyFloat = true;
            break;
        }
    }
    if(anyFloat){
        f64 res = ELux::as_float(args[0]);
        if(args.size() == 1){
            return std::make_shared<Number>(-res);
        }
        for(size_t i = 1; i < args.size(); ++i){
            res -= ELux::as_float(args[i]);
        }
        return std::make_shared<Number>(res);
    }else{
        auto res = ELux::as_integer(args[0]);
        if(args.size() == 1){
            return std::make_shared<Number>(-res);
        }
        for(size_t i = 1; i < args.size(); ++i){
            res -= ELux::as_integer(args[i]);
        }
        return std::make_shared<Number>(res);
    }
}

// -*-
Self mul(const Vec<Self>& args) {
    if(args.empty()){return std::make_shared<Number>(i64(1)); }
    bool anyFloat = false;
    for(auto& a : args){
        if(ELux::is_float(a)){
            anyFloat = true;
            break;
        }
    }
    if(anyFloat){
        f64 res = 1.0;
        for(auto& a : args){ res *= ELux::as_float(a); }
        return std::make_shared<Number>(res);
    }else{
        i64 res = 1;
        for(auto& a : args){ res *= ELux::as_integer(a); }
        return std::make_shared<Number>(res);
    }
}

// -*-
Self divv(const Vec<Self>& args) {
    if(args.empty()){
        throw std::runtime_error("`/' needs at least one arg");
    }
    auto res = ELux::as_float(args[0]);
    if(args.size() == 1){
        if(res==0.0){ throw std::runtime_error("division by zero"); }
        return std::make_shared<Number>((1.0/res));
    }
    for(size_t i = 1; i < args.size(); ++i){
        auto deno = ELux::as_float(args[i]);
        if(deno==0.0){ throw std::runtime_error("division by zero"); }
        res /= deno;
    }
    return std::make_shared<Number>(res);
}

// -*-
Self modfn(const Vec<Self>& args){
    if(args.empty()){
        throw std::runtime_error("`%' needs at least one arg");
    }
    auto res = ELux::as_float(args[0]);
    if(args.size() == 1){
        if(res==0.0){ throw std::runtime_error("division by zero"); }
        return std::make_shared<Number>(std::fmod(res, 1.0));
    }
    for(size_t i = 1; i < args.size(); ++i){
        auto deno = ELux::as_float(args[i]);
        if(deno==0.0){ throw std::runtime_error("division by zero"); }
        res = std::fmod(res, deno);
    }
    return std::make_shared<Number>(res);
}

// -*-
Self eq(const Vec<Self>& args){
    if(args.size() < 2){ return std::make_shared<Bool>(true); }
    for(size_t i = 1; i < args.size(); ++i){
        if(args[0]->type()=="Function" || args[i]->type()=="Function"){
            return std::make_shared<Bool>(false);
        }
        if(args[i]->str() != args[0]->str()){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self lt(const Vec<Self>& args) {
    if(args.size() < 2){
        return std::make_shared<Bool>(true);
    }
    for(size_t i = 1; i < args.size(); ++i){
        if(!(ELux::as_float(args[i-1]) <  ELux::as_float(args[i]))){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self lte(const Vec<Self>& args) {
    if(args.size() < 2){
        return std::make_shared<Bool>(true);
    }
    for(size_t i = 1; i < args.size(); ++i){
        if(!(ELux::as_float(args[i-1]) <= ELux::as_float(args[i]))){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self gt(const Vec<Self>& args) {
    if(args.size() < 2){
        return std::make_shared<Bool>(true);
    }
    for(size_t i = 1; i < args.size(); ++i){
        if(!(ELux::as_float(args[i-1]) > ELux::as_float(args[i]))){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self gte(const Vec<Self>& args){
    if(args.size() < 2){
        return std::make_shared<Bool>(true);
    }
    for(size_t i = 1; i < args.size(); ++i){
        if(!(ELux::as_float(args[i-1]) >= ELux::as_float(args[i]))){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self logical_and(const Vec<Self>& args){
    for(auto& a : args){
        if(!ELux::as_bool(a) || ELux::is_nil(a)){
            return std::make_shared<Bool>(false);
        }
    }
    return std::make_shared<Bool>(true);
}

Self logical_or(const Vec<Self>& args){
    for(auto& a : args){
        if(ELux::as_bool(a)){
            return std::make_shared<Bool>(true);
        }
    }
    return std::make_shared<Bool>(false);
}

Self logical_not(const Vec<Self>& args){
    if(args.size() != 1){
        throw std::runtime_error("not expects 1 arg");
    }
    return std::make_shared<Bool>(!ELux::as_bool(args[0]));
}

Self bit_and(const Vec<Self>& args){
    if(args.empty()){
        return std::make_shared<Number>(i64(-1));
    }
    auto res = ELux::as_integer(args[0]);
    for(size_t i = 1; i < args.size(); ++i){
        res &= ELux::as_integer(args[i]);
    }
    return std::make_shared<Number>(res);
}

Self bit_or(const Vec<Self>& args) {
    if(args.empty()){
        return std::make_shared<Number>();
    }
    auto res = ELux::as_integer(args[0]);
    for(size_t i = 1; i < args.size(); ++i){
        res |= ELux::as_integer(args[i]);
    }
    return std::make_shared<Number>(res);
}

Self bit_xor(const Vec<Self>& args) {
    if(args.empty()){
        return std::make_shared<Number>();
    }
    auto res = ELux::as_integer(args[0]);
    for(size_t i = 1; i < args.size(); ++i){
        res ^= ELux::as_integer(args[i]);
    }
    return std::make_shared<Number>(res);
}

Self bit_not(const Vec<Self>& args) {
    if(args.size() != 1){
        throw std::runtime_error("bit-not expects 1 arg");
    }
    return std::make_shared<Number>(~ELux::as_integer(args[0]));
}

// =========================
// Functional builtins
// =========================
// -*-
Self builtin_map(const Vec<Self>& args, Context env){
    if(args.size() != 2){
        throw std::runtime_error("`map' expects function and list");
    }
    auto fnVal = args[0];
    if(!ELux::is_function(fnVal) && !ELux::is_macro(fnVal)){
        throw std::runtime_error("`map' first arg must be function");
    }
    auto fn = dynamic_cast<Function*>(fnVal.get());
    auto listVal = args[1];
    // -
    auto notOk = (
        !ELux::is_list(listVal) &&
        !ELux::is_array(listVal) &&
        !ELux::is_string(listVal) &&
        !ELux::is_set(listVal) &&
        !ELux::is_dict(listVal)
    );
    if(notOk){
        throw std::runtime_error("map expects list/array/string/set/dict");
    }
    Self result = ELux::share();
    ELux elux;
    if(ELux::is_list(listVal)){
        List ans;
        const auto& xs = *dynamic_cast<List*>(listVal.get());;
        for (auto& item : xs.value()){
            Vec<Self> callArgs = { item };
            if (fn->isNative) {
                ans.value().push_back(fn->native(callArgs, env));
            } else {
                auto callEnv = std::make_shared<Env>(fn->closure);
                if (fn->params.size() != 1){
                    throw std::runtime_error("map function must take 1 arg");
                }
                callEnv->define(fn->params[0], item);
                ans.value().push_back(fn->body->eval(elux, callEnv));
            }
        }
        result = ELux::share(ans);
    }else if(ELux::is_array(listVal)) {
        Array ans{};
        const Array& xs = *dynamic_cast<Array*>(listVal.get());
        for (auto& item : xs.value()){
            std::vector<Self> callArgs = { item };
            if(fn->isNative){
                ans.value().push_back(fn->native(callArgs, env));
            }else{
                auto callEnv = std::make_shared<Env>(fn->closure);
                if(fn->params.size() != 1){
                    throw std::runtime_error("map function must take 1 arg");
                }
                callEnv->define(fn->params[0], item);
                ans.value().push_back(fn->body->eval(elux, callEnv));
            }
        }
        result = ELux::share(ans);
    }else if(ELux::is_string(listVal)){
        String ans{};
        const auto& xstr = dynamic_cast<String*>(listVal.get());
        for(auto& c : xstr->str()){
            Vec<Self> callArgs = { std::make_shared<String>(c) };
            if(fn->isNative){
                auto rv = fn->native(callArgs, env);
                if(!ELux::is_string(rv)){
                    throw std::runtime_error(
                        "`map' applied to string. The callable argument must return a string"
                    );
                }
                ans.value() += dynamic_cast<String*>(rv.get())->value();
            } else {
                auto callEnv = std::make_shared<Env>(fn->closure);
                if (fn->params.size() != 1){
                    throw std::runtime_error("map function must take 1 arg");
                }
                callEnv->define(fn->params[0], ELux::share(c));
                auto rv = fn->body->eval(elux, callEnv);
                if(ELux::is_string(rv)){
                    throw std::runtime_error(
                        "`map' applied to string. The callable argument must return a string"
                    );
                }
                ans.value() += dynamic_cast<String*>(fn->body->eval(elux, callEnv).get())->value();
            }
        }
        result = ELux::share(ans);
    }else if(ELux::is_set(listVal)){
        Set ans{};
        const auto& xset = dynamic_cast<Set*>(listVal.get())->value();
        for (auto& item : xset) {
            Vec<Self> callArgs = { ELux::share(item) };
            if (fn->isNative){
                auto rv = fn->native(callArgs, env);
                if(!ELux::is_string(rv)){
                    throw std::runtime_error(
                        "`map' applied to set. The callable argument must return a string."
                    );
                }
                ans.value().insert(rv->str());
            } else{
                auto callEnv = std::make_shared<Env>(fn->closure);
                if (fn->params.size() != 1){
                    throw std::runtime_error("map function must take 1 arg");
                }
                callEnv->define(fn->params[0], ELux::share(item));
                auto rv = fn->body->eval(elux, callEnv);
                if(!ELux::is_string(rv)){
                    throw std::runtime_error(
                        "`map' applied to set. The callable argument must return a string."
                    );
                }
                ans.value().insert(rv->str());
            }
        }
        result = ELux::share(ans);
    }else{
        Dict ans{};
        const auto& xdict = ELux::as_dict(listVal);
        for (auto& item : xdict.value()){
            Array entry{};
            entry.value().push_back(ELux::share(item.first));
            entry.value().push_back(item.second);
            Vec<Self> callArgs = { ELux::share(entry) };
            if (fn->isNative){
                auto xs = fn->native(callArgs, env);
                if(!ELux::is_array(xs)){
                    std::stringstream ss;
                    ss << "`map' applied to dict. Expect the callable argument to return\n";
                    ss << "an array with two elements.";
                    throw std::runtime_error(ss.str());
                }
                auto rv = ELux::as_array(xs);
                if(rv.value().size() != 2 || !ELux::is_string(rv.value()[0])){
                    std::stringstream ss;
                    ss << "`map' applied to dict. Expect the callable argument to return\n";
                    ss << "an array with two elements.";
                    throw std::runtime_error(ss.str());
                }
                auto key = ELux::str(rv.value()[0]);
                ans.value()[key] = rv.value()[1];
            } else {
                auto callEnv = std::make_shared<Env>(fn->closure);
                if(fn->params.size() != 1){
                    throw std::runtime_error("map function must take 1 arg");
                }
                callEnv->define(fn->params[0], ELux::share(entry));
                auto xs = fn->body->eval(elux, callEnv);
                if(!ELux::is_array(xs)){
                    std::stringstream ss;
                    ss << "`map' applied to dict. Expect the callable argument to return\n";
                    ss << "an array with two elements.";
                    throw std::runtime_error(ss.str());
                }
                auto rv = ELux::as_array(xs);
                if(rv.value().size() != 2 || !ELux::is_string(rv.value()[0])){
                    std::stringstream ss;
                    ss << "`map' applied to dict. Expect the callable argument to return\n";
                    ss << "an array with two elements.";
                    throw std::runtime_error(ss.str());
                }
                auto key = ELux::str(rv.value()[0]);
                ans.value()[key] = rv.value()[1];
            }
        }
        result = ELux::share(ans);
    }
    return result;
}

Self builtin_filter(const Vec<Self>& args, Context env){
    if(args.size() != 2){
        throw std::runtime_error("filter expects function and list");
    }
    auto fnVal = args[0];
    if(!ELux::is_function(fnVal) && !ELux::is_macro(fnVal)){
        throw std::runtime_error("filter first arg must be function");
    }
    auto fn = ELux::as_function(fnVal);
    auto listVal = args[1];
    auto notOk = (
        !ELux::is_list(listVal) &&
        !ELux::is_array(listVal) &&
        !ELux::is_string(listVal) &&
        !ELux::is_set(listVal) &&
        !ELux::is_dict(listVal)
    );
    if(notOk){
        throw std::runtime_error("filter expects list/array/string/set/dict");
    }
    Self result = ELux::share();
    ELux elux;
    auto processItem = [&](const Self& item){
        Vec<Self> callArgs = { item };
        Self r;
        if(fn.isNative){
            r = fn.native(callArgs, env);
        }else{
            auto callEnv = std::make_shared<Env>(fn.closure);
            if(fn.params.size() != 1){
                throw std::runtime_error("filter function must take 1 arg");
            }
            callEnv->define(fn.params[0], item);
            r = fn.body->eval(elux, callEnv);
        }
        if(ELux::as_bool(r)){ return item; }
        return ELux::share();
    };

    if(ELux::is_list(listVal)){
        List ans{};
        const auto& xs = ELux::as_list(listVal);
        for(auto& item : xs.value()){
           auto self = processItem(item);
           if(!ELux::is_nil(self)){ ans.value().push_back(self); }
        }
        result = ELux::share(ans);
    }else if(ELux::is_array(listVal)){ //!
        Array ans{};
        const auto& xs = ELux::as_array(listVal);
        for(auto& item : xs.value()){
           auto self = processItem(item);
           if(!ELux::is_nil(self)){ ans.value().push_back(self); }
        }
        result = ELux::share(ans);
    }else if(ELux::is_string(listVal)){ //!
        std::string ans{};
        const auto& xs = ELux::str(listVal);
        auto pos = xs.find(' ');
        decltype(pos) idx = 0;
        while(pos != std::string::npos){
            auto token = xs.substr(idx, pos);
            auto item = ELux::share(token);
            auto self = processItem(item);
            if(ELux::is_nil(self)){ continue; }
            if(!ELux::is_string(self)){
                throw std::runtime_error("expect string");
            }
            auto str = ELux::str(self);
            ans += str;
            auto i=pos;
            while(i < xs.length()){
                auto c = xs[i];
                if(!std::isspace(c)){ break; }
                if(c=='\t'){ ans += '\t'; }
                else if(c=='\n'){ ans += '\n'; }
                else{ ans += ' '; }
            }
            idx = i;
            pos = xs.find(' ', idx);
        }
        ans += xs.substr(idx);
        result = ELux::share(ans);
    }else if(ELux::is_set(listVal)){ //!
        Set ans{};
        const auto& xs = ELux::as_set(listVal);
        for(auto& item : xs.value()){
            auto self = processItem(ELux::share(item));
            if(ELux::is_nil(self)){ continue; }
            if(!ELux::is_string(self)){
                std::stringstream ss;
                ss << "`filter' applied to set. Expect the callable argument to\n";
                ss << "return a string.";
                throw std::runtime_error(ss.str());
            }
            ans.value().insert(ELux::str(self));
        }
        result = ELux::share(ans);
    }else{ // Dict
        Dict ans{};
        const auto& xdict = ELux::as_dict(listVal);
        for(auto& item : xdict.value()){
            Array xs{};
            xs.value().push_back(ELux::share(item.first));
            xs.value().push_back(item.second);
            auto self = processItem(ELux::share(xs));
            if(ELux::is_nil(self)){ continue; }
            if(!ELux::is_array(self)){
                std::stringstream ss;
                ss << "`filter' applied to dict. Expected the callable argument to\n";
                ss << "return an array of 2 elements, where the first element must be\n";
                ss << "a string.";
                throw std::runtime_error(ss.str());
            }
            auto vec = ELux::as_array(self);
            if(vec.value().size() != 2 || !ELux::is_string(vec.value()[0])){
                std::stringstream ss;
                ss << "`filter' applied to dict. Expected the callable argument to\n";
                ss << "return an array of 2 elements, where the first element must be\n";
                ss << "a string.";
                throw std::runtime_error(ss.str());
            }
            auto key = ELux::str(vec.value()[0]);
            ans.value()[key] = vec.value()[1];
        }
        result = ELux::share(ans);
    }
    return result;
}

// -*-
Self builtin_reduce(const Vec<Self>& args, Context env){
    if(args.size() != 3){
        throw std::runtime_error("reduce expects function, init, list");
    }
    auto fnVal = args[0];
    if(!ELux::is_callable(fnVal)){
        throw std::runtime_error("reduce first arg must be function");
    }
    auto fn = ELux::as_function(fnVal);
    auto acc = args[1];
    auto listVal = args[2];
    auto notOk = (
        !ELux::is_list(listVal) &&
        !ELux::is_array(listVal) &&
        !ELux::is_set(listVal) &&
        !ELux::is_dict(listVal)
    );
    if(notOk){
        throw std::runtime_error("reduce expects list/array/set/dict");
    }
    ELux elux;
    auto processItem = [&](const Self& item) {
        Vec<Self> callArgs = { acc, item };
        if(fn.isNative){
            acc = fn.native(callArgs, env);
        }else{
            auto callEnv = std::make_shared<Env>(fn.closure);
            if(fn.params.size() != 2){
                throw std::runtime_error("reduce function must take 2 args");
            }
            callEnv->define(fn.params[0], acc);
            callEnv->define(fn.params[1], item);
            acc = fn.body->eval(elux, callEnv);
        }
    };
    if(ELux::is_list(listVal)){
        const auto& xs = ELux::as_list(listVal);
        for (auto& item : xs.value()){ processItem(item); }
    }else if(ELux::is_array(listVal)){
        const auto& xs = ELux::as_array(listVal);
        for (auto& item : xs.value()){ processItem(item); }
    }else if(ELux::is_set(listVal)){
        const auto& xs = ELux::as_set(listVal);
        for (auto& item : xs.value()){
            processItem(ELux::share(item));
        }
    }else{
        const auto& xs = ELux::as_dict(listVal);
        for(auto& item : xs.value()){
            Array vec{};
            vec.value().push_back(ELux::share(item.first));
            vec.value().push_back(item.second);
            processItem(ELux::share(vec));
        }
    }
    return acc;
}

// -*-
Self builtin_zip(const Vec<Self>& args, Context env){
    if (args.size() < 2) throw std::runtime_error("zip expects at least 2 lists");
    size_t minLen = std::numeric_limits<size_t>::max();
    //std::vector<const List*> lists;
    using Pair = std::pair<bool, const Object*>;
    Vec<Pair> vec{};
    for(const auto& val : args){
        if((!(ELux::is_list(val) || ELux::is_array(val)))){
            throw std::runtime_error("zip expects lists and/or arrays");
        }
        if(ELux::is_list(val)){
            List* ptr = reinterpret_cast<List*>(val.get());
            auto n = ptr->value().size();
            vec.push_back({true, ptr});
            minLen = std::min(minLen, n);
        }else{
            auto ptr = reinterpret_cast<Array*>(val.get());
            auto n = ptr->value().size();
            vec.push_back({false, ptr});
            minLen = std::min(minLen, n);
        }
        // const List& l = std::get<List>(v.data);
        // lists.push_back(&l);
        // minLen = std::min(minLen, l.size());
    }
    //List result;
    Array result;
    for (size_t i = 0; i < minLen; ++i) {
        Array tuple;
        for(const auto& arg : vec){
            if(arg.first){
                auto& xs = reinterpret_cast<const List*>(arg.second)->value();
                auto ptr = xs.begin(); //std::get<List>(arg.data).begin();
                std::advance(ptr, i);
                tuple.value().push_back(*ptr);
            }else{
                auto& xs = reinterpret_cast<const Array*>(arg.second)->value();
                tuple.value().push_back(xs[i]);
            }
        }
        result.value().push_back(ELux::share(tuple));
    }
    return ELux::share(result);
}

// -*-
Self builtin_enumerate(const Vec<Self>& args, Context env){
    if (args.size() != 1) throw std::runtime_error("enumerate expects list");
    auto listVal = args[0];
    auto failed = (
        !ELux::is_list(listVal) &&
        !ELux::is_array(listVal) &&
        !ELux::is_string(listVal) &&
        !ELux::is_set(listVal) &&
        !ELux::is_dict(listVal)
    );
    if(failed){
        throw std::runtime_error("enumerate expects list/array/string/set/dict");
    }
    Array result;
    size_t idx = 0;
    if(ELux::is_list(listVal)){
        const auto& xs = ELux::as_list(listVal);
        for(auto& item : xs.value()){
            Array pair;
            pair.value().push_back(ELux::share(static_cast<i64>(idx++)));
            pair.value().push_back(item);
            result.value().push_back(ELux::share(pair));
        }
    }else if(ELux::is_array(listVal)){
        const auto& xs = ELux::as_array(listVal);
        for (auto& item : xs.value()){
            Array pair;
            pair.value().push_back(ELux::share(static_cast<i64>(idx++)));
            pair.value().push_back(item);
            result.value().push_back(ELux::share(pair));
        }
    }else if(ELux::is_string(listVal)){
        const auto& xs = ELux::as_string(listVal);
        for (auto& item : xs.value()) {
            Array pair;
            pair.value().push_back(ELux::share(static_cast<i64>(idx++)));
            pair.value().push_back(ELux::share(item));
            result.value().push_back(ELux::share(pair));
        }
    }else if(ELux::is_set(listVal)){
        const auto& xs = ELux::as_set(listVal);
        for (auto& item : xs.value()) {
            Array pair;
            pair.value().push_back(ELux::share(static_cast<i64>(idx++)));
            pair.value().push_back(ELux::share(item));
            result.value().push_back(ELux::share(pair));
        }
    }else{ // Dict
        const auto& xs = ELux::as_dict(listVal);
        for (auto& item : xs.value()){
            Array pair;
            Array self{};
            self.value().push_back(ELux::share(item.first));
            self.value().push_back(item.second);
            pair.value().push_back(ELux::share(static_cast<i64>(idx++)));
            pair.value().push_back(ELux::share(self));
            result.value().push_back(ELux::share(pair));
        }
    }
    return ELux::share(result);
}

// -*-
Self builtin_repr(const Vec<Self>& args, Context env){
    std::stringstream ss;
    auto idx = 0;
    for(auto self: args){
        if(idx > 0){ ss << " "; }
        ss << ELux::repr(self);
    }

    return ELux::share(ss.str());
}


// =========================
// Global environment setup
// =========================
// -*-
void addNative(Context env, const std::string& name,
               std::function<Self(const Vec<Self>& args, Context env)> fn) {
    auto fun = std::make_shared<Function>();
    fun->isNative = true;
    fun->native = std::move(fn);
    env->define(name, fun);
}

// -*-
Context makeGlobalEnv() {
    auto env = std::make_shared<Env>();

    addNative(env, "+", [](const Vec<Self>& a, Context){
        return add(a); }
    );
    addNative(env, "-", [](const Vec<Self>& a, Context){
        return sub(a); }
    );
    addNative(env, "*", [](const Vec<Self>& a, Context){
        return mul(a); }
    );
    addNative(env, "/", [](const Vec<Self>& a, Context){
        return divv(a); }
    );
    addNative(env, "%", [](const Vec<Self>& a, Context){
        return modfn(a); }
    );

    addNative(env, "=", [](const Vec<Self>& a, Context){
        return eq(a); }
    );
    addNative(env, "<", [](const Vec<Self>& a, Context){
        return lt(a); }
    );
    addNative(env, "<=", [](const Vec<Self>& a, Context){
        return lte(a); }
    );
    addNative(env, ">", [](const Vec<Self>& a, Context){
        return gt(a); }
    );
    addNative(env, ">=", [](const Vec<Self>& a, Context){
        return gte(a); }
    );

    addNative(env, "and", [](const Vec<Self>& a, Context){
        return logical_and(a); }
    );
    addNative(env, "or", [](const Vec<Self>& a, Context){
        return logical_or(a); }
    );
    addNative(env, "not", [](const Vec<Self>& a, Context){
        return logical_not(a); }
    );

    addNative(env, "&", [](const Vec<Self>& a, Context){
        return bit_and(a); }
    );
    addNative(env, "|", [](const Vec<Self>& a, Context){
        return bit_or(a); }
    );
    addNative(env, "^", [](const Vec<Self>& a, Context){
        return bit_xor(a); }
    );
    addNative(env, "~", [](const Vec<Self>& a, Context){
        return bit_not(a); }
    );

    addNative(env, "List.new", [](const Vec<Self>& args, Context){
        List result{};
        for(auto arg: args){
            result.value().push_back(arg);
        }
        return ELux::share(result);
    });

    addNative(env, "Array.new", [](const Vec<Self>& a, Context){
        Array arr{};
        arr.value().insert(arr.value().begin(), a.begin(), a.end());
        return ELux::share(arr);
    });

    addNative(env, "Set.new", [](const Vec<Self>& a, Context){
        Array arr{};
        arr.value().insert(arr.value().begin(), a.begin(), a.end());
        Set xset(arr);
        return ELux::share(xset);
    });

    addNative(env, "Dict.new", [](const Vec<Self>& args, Context){
        if(args.size() % 2 != 0){
            throw std::runtime_error("dict expects even number of args");
        }
        Dict dict;
        for (size_t i = 0; i < args.size(); i += 2) {
            if(!ELux::is_string(args[i]))
                throw std::runtime_error("dict keys must be strings");
            dict.value()[ELux::str(args[i])] = args[i+1];
        }
        return ELux::share(dict);
    });

    addNative(env, "get", [](const Vec<Self>& args, Context){
        if(args.size() != 2){
            throw std::runtime_error("get expects dict and key");
        }
        auto failed = (
            !ELux::is_dict(args[0]) &&
            !ELux::is_array(args[0]) &&
            !ELux::is_list(args[0]) &&
            !ELux::is_string(args[0])
        );
        if(failed){
            throw std::runtime_error("get expects dict/array/list/string");
        }
        auto result = ELux::share();
        if(ELux::is_dict(args[0])){
            if(!ELux::is_string(args[1])){
                throw std::runtime_error("get key must be string when applied to dict.");
            }
            const Dict& dict = ELux::as_dict(args[0]);
            auto key = ELux::str(args[1]);
            auto it = dict.value().find(key);
            if(it != dict.value().end()){
                result = it->second;
            }else{
                std::stringstream ss;
                ss << "key " << std::quoted(key) << " not found.";
                throw std::runtime_error(ss.str());
            }
        }else if(ELux::is_array(args[0])){
            if(ELux::is_integer(args[1])){
                throw std::runtime_error("get key must be integer when applied to array.");
            }
            auto idx = ELux::as_integer(args[1]);
            auto self = ELux::as_array(args[0]);
            if(idx >=0 && idx < self.value().size()){
                result = self.value()[idx];
            }else{
                std::stringstream ss;
                ss << "index " << idx << " out of range.";
                throw std::runtime_error(ss.str());
            }
        }else if(ELux::is_list(args[0])){
            if(!ELux::is_integer(args[1])){
                throw std::runtime_error("get key must be integer when applied to list.");
            }
            auto idx = ELux::as_integer(args[1]);
            auto self = ELux::as_list(args[0]);
            if(idx >=0 && idx < self.value().size()){
                auto ptr = self.value().begin();
                std::advance(ptr, idx);
                result = *ptr;
            }else{
                std::stringstream ss;
                ss << "index " << idx << " out of range.";
                throw std::runtime_error(ss.str());
            }
        }else{
            if(!ELux::is_integer(args[1])){
                throw std::runtime_error("get key must be integer when applied to string.");
            }
            auto idx = ELux::as_integer(args[1]);
            auto self = ELux::as_string(args[0]);
            if(idx >=0 && idx < self.value().length()){
                result = ELux::share(self.value()[idx]);
            }else{
                std::stringstream ss;
                ss << "index " << idx << " out of range.";
                throw std::runtime_error(ss.str());
            }
        }
        
        return result;
    });

    // push, len

    addNative(env, "println", [](const Vec<Self>& args, Context) {
        for(size_t i = 0; i < args.size(); ++i){
            if(i){ std::cout << " "; }
            std::cout << ELux::str(args[i]);
        }
        std::cout << std::endl;
        return ELux::share();
    });

    addNative(env, "eprintln", [](const Vec<Self>& args, Context) {
        for(size_t i = 0; i < args.size(); ++i){
            if(i){ std::cout << " ";}
            std::cerr << ELux::str(args[i]);
        }
        std::cerr << std::endl;
        return ELux::share();
    });

    addNative(env, "print", [](const Vec<Self>& args, Context){
        for(size_t i = 0; i < args.size(); ++i) {
            if(i){ std::cout << " "; }
            std::cout << ELux::str(args[i]);
        }
        return ELux::share();
    });

    addNative(env, "eprint", [](const Vec<Self>& args, Context){
        for(size_t i = 0; i < args.size(); ++i){
            if(i){ std::cerr << " "; }
            std::cerr << ELux::str(args[i]);
        }
        return ELux::share();
    });

    addNative(env, "panic", [](const Vec<Self>& args, Context){
        for(size_t i = 0; i < args.size(); ++i){
            if(ELux::is_nil(args[i])){ continue; }
            if(i){ std::cout << " "; }
            std::cerr << ELux::str(args[i]);
        }
        std::exit(EXIT_FAILURE);
        return ELux::share();
    });

    addNative(env, "quit", [](const Vec<Self>& args, Context ctx){
        [[maybe_unused]] auto _ = args;
        [[maybe_unused]] auto __ = ctx;
        std::exit(EXIT_SUCCESS);
        return ELux::share();
    });

    addNative(env, "bye", [](const Vec<Self>& args, Context ctx){
        if(args.size() > 1){
            throw std::runtime_error("`byte': too may arguments. Expect at most 1 arguments");
        }
        if(args.size()==1){
            if(!ELux::is_string(args[0])){
                std::stringstream ss;
                ss << "`bye': expected a string as argument but got ";
                ss << std::quoted(args[0]->type()) << ".";
                throw std::runtime_error(ss.str());
            }
            std::cout << ELux::str(args[0]) << std::endl;
        }
        std::exit(EXIT_SUCCESS);
        return ELux::share();
    });

    addNative(env, "exit", [](const Vec<Self>& args, Context ctx){
        if(args.size() != 1){
            throw std::runtime_error("`exit': invalid number of arguments. Expects 1 integer argument.");
        }
        if(!ELux::is_integer(args[0])){
            std::stringstream ss;
            ss << "`exit': invalid argument type. Expect an Integer but got ";
            ss << args[0]->type() << ".";
            throw std::runtime_error(ss.str());
        }
        
        std::exit(ELux::as_integer(args[0]));
        return ELux::share();
    });

    addNative(env, "len", [](const Vec<Self>& args, Context ctx){
        if(args.size() != 1){
            throw std::runtime_error("`len': invalid number of arguments. Expects 1 integer argument.");
        }
        
        return ELux::share(ELux::len(args[0]));
    });

    //addNative(env, "repr", builtin_repr);
    addNative(env, "map", builtin_map);
    addNative(env, "filter", builtin_filter);
    addNative(env, "reduce", builtin_reduce);
    addNative(env, "zip", builtin_zip);
    addNative(env, "enumerate", builtin_enumerate);
    env->define("true", ELux::share(true));
    env->define("false", ELux::share(false));
    env->define("Math.E", ELux::share(2.718281828459045));
    env->define("Math.PI", ELux::share(3.141592653589793));
    // NaN

    // min, max, linspace, range, random
    // abs,
    // floor, round, ceil, truncate
    // cos, sin, tan, acos, asin, atan, atan2
    // cosh, sinh, tanh, acosh, asinh, atanh
    // exp, exp2, expm1
    // log, log2, log10, log1p
    // erf, erfc, lgamma, tgamma
    // isinf, isfinite, isnan,
    //
    // addNative(env, "panic", [](const std::vector<Value>& a, std::shared_ptr<Env>) {
    //      //! @todo
    // });

    return env;
}


// -*----------------------------------------------------------------*-
}//-*- end::namespace::klx                                          -*-
// -*----------------------------------------------------------------*-