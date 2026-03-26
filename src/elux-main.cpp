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
    //auto prelude = std::make_shared<ekasoft::klx::Env>();
    //prelude->vars = global;
    repl(global);

    // // Example module file: write to disk for import demo
    // {
    //     std::ofstream mod("mymath.elux");
    //     mod << R"ELUX(
    //         (define pi 3.14159)
    //         (fun square (x) (* x x))
    //         (define exports (List.new "pi" "square"))
    //     )ELUX";
    //     mod.close();
    // }

    // // 1. Basic arithmetic, logical, bitwise
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn 
    //             (println "(+ 1 2 3)     => " (+ 1 2 3))
    //             (println "(and t nil t) => " (and t nil t))
    //             (println "(& 7 3)       => " (& 7 3))
    //             (println "(+ 10 20)     => " (+ 10 20)))
    //     )ELUX",
    //     global, "Arithmetic / logical / bitwise");

    // // 2. defvar, setq, if, while, for
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (var x 0)
    //             (while (< x 3)
    //                 (println x)
    //                 (var x (+ x 1)))
    //             (var sum 0)
    //             (for (i (List.new 1 2 3 4))
    //                 (var sum (+ sum i)))
    //             (println sum))
    //     )ELUX",
    //     global, "Variables / loops");

    // // 3. lambda, defun, map, filter, reduce, enumerate, zip
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (fun inc (x) (+ x 1))
    //             (var xs (List.new 1 2 3 4))
    //             (println (map inc xs))
    //             (println (filter (lambda (x) (> x 2)) xs))
    //             (println (reduce + 0 xs))
    //             (println (enumerate xs))
    //             (println (zip xs (List.new 10 20 30 40)))
    //             (reduce + 0 xs))
    //     )ELUX",
    //     global, "Functional programming");

    // // 4. cond, match, try/catch
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (print (cond
    //                     ((< 1 0) (println "no"))
    //                     ((= 1 1) (println "yes"))
    //                     (t (println "else"))))
    //             (print (match 2
    //                         (1 (println "one"))
    //                         (2 (println "two"))
    //                         (t (println "other"))))
    //             (try
    //                 (progn
    //                     (println (/ 1 0))
    //                     (println "ok"))
    //                 (catch err
    //                     (eprintln "caught" err)
    //                     (println "handled"))))
    //     )ELUX",
    //     global, "cond / match / try-catch");

    // // 5. quote, quasiquote, unquote, unquote-splicing
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (var a 10)
    //             (var b (List.new 1 2 3))
    //             (println (quote (a b c)))
    //             (println `(+ ,a 20))
    //             (println `(1 2 ,@b 4))
    //             (println `(foo ,a ,@b)))
    //     )ELUX",
    //     global, "quote / quasiquote / unquote / unquote-splicing");

    // // 6. defmacro demonstration
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (macro unless (cond body)
    //                 `(if (not ,cond) ,body nil))
    //             (var x 0)
    //             (unless (= x 1)
    //             (var x 42))
    //             (println x))
    //     )ELUX",
    //     global, "defmacro / macro expansion");

    // // 7. Module system with import and exports
    // ekasoft::klx::ELux::run(
    //     R"ELUX(
    //         (progn
    //             (define mathlib (import "mymath.elux"))
    //             (println (get mathlib "pi"))
    //             (println ((get mathlib "square") 5))
    //             (println (get mathlib "square") 3))
    //     )ELUX",
    //     global, "Modules / import / exports");

    return 0;
}