#include "elux.hpp"
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