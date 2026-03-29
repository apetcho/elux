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
#include<stack>

struct Reader{
    std::string read(void){
        char c{};
        std::cin >> c;
        std::cin.unget();
        if(c=='('){
            return this->read_list();
        }
        return this->read_literal();
    }

private:
    std::string read_literal(void){
        std::string data;
        std::getline(std::cin, data);
        return data;
    }

    std::string read_list(void){
        std::stringstream ss;
        std::stack<char> parens{};
        while(true){
            std::string line{};
            std::getline(std::cin, line);
            ss << line;
            for(auto c: line){
                if(c=='('){ parens.push(c); }
                if(c==')'){ parens.pop(); }
            }
            if(parens.empty()){ break; }
        }
        
        return ss.str();
    }
};

// -*-
void repl(std::shared_ptr<ekasoft::klx::Env> env){
    size_t id = 0;
    auto prompt = [&id](){
        std::cout << "\x1b[32melux\x1b[0m:\x1b[33m" << ++id << "\x1b[0m>> ";
    };
    Reader reader;
    auto runtime = std::make_shared<ekasoft::klx::Env>(env);
    while(true){
        prompt();
        auto src = reader.read();
        //std::cerr << "SOURCE:\n" << src << std::endl;
        if(src=="quit"){ break; }
        try{
            ekasoft::klx::ELux elux;
            ekasoft::klx::Parser parser(src);
            auto exprs = parser.parse();
            ekasoft::klx::Self result{};
            for(auto expr: exprs){
                result = elux.eval(expr, runtime);
            }
            if(!ekasoft::klx::ELux::is_nil(result)){
                std::cout << ekasoft::klx::ELux::str(result) << std::endl;
            }
        }catch(const std::runtime_error& err){
            std::cerr << "\x1b[31mELuxError\x1b[0m: " << err.what() << std::endl;
        }catch(const std::exception& err){
            std::cerr << "\x1b[31mUnexpectedError\x1b[0m: " << err.what() << std::endl;
        }
    }
}


int main(){
    auto global = ekasoft::klx::makeGlobalEnv();
    repl(global);
    return 0;
}