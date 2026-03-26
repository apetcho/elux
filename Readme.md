<div style="display: inline-block; text-align: center;">
    <img src="./logo/elux.png">
</div>

# ELux Programming language

ELux is a lisp-like programming language. The implementation is a work in progress.
It is an interpreted language implemented in C++.

## How to test the language.

```shell
$ git clone ...
$ cd elux
$ cd build
$ cmake ..
$ cmake --build .
$ ./src/ELux
elux:1>> (print "Welcome to ELux land\n")
```

## Quickstart

To define a varibale *var* and *define* to define a constant.

```elux
elux:1>> (var x 10)
10
elux:2>> (define VERSION "0.0.1")
0.0.1
```

Here is a simple "Hello world!"

```elux
elux:3>> (println "Hello world!")
```

Let do some basic arithmetic.

```elux
elux:4>> (+ 1 2 3 4)
10
elux:5>> (* 3 6)
18
elux:6>> (/ 5 2)
2.5
elux:6>> (- 12 10)
2
elux:7>> (% 29 3)
2
```

ELux can define function using *fun* and anonymous function using *lambda*

```elux
elux:8>> (fun hello(name)
    (println "Hello " name))
elux:9>> (hello "World!")
Hello World!
elux:10>> (var xval ((lambda (x) (* x 113)) 32))
3616
```

ELux provides *List*, *Array*, *Set* and *Dict* data structures. It also support
working with *String*. Let create some data structures.

```elux
elux:11>> (var languages (List.new "Rust" "C++" "Python" "Javascript" "Lisp"))
(Rust C++ Python Javascript Lisp)
elux::12>> languages
(Rust C++ Python Javascript Lisp)
elux::13>> (var data (Array.new 1 2 3 4 5 6 7))
[1 2 3 4 5 6 7]
elux:14>> (var shopping-list (Set.new "Tomato" "Pasta" "Eggs" "Milk"))
#{Eggs Milk Pasta Tomato}
elux:15>> (var engineers (Dict.new "McCarthy" "Lisp" "D. Ritchie" "C" "B. Stroustrop" "C++" "G. van Rossum" "Python" "B. Eich" "Javascript"))
{[B. Eich Javascript] [B. Stroustrop C++] [D. Ritchie C] [G. van Rossum Python] [McCarthy Lisp]}
elux:16>> (len engineers)
5
elux:17>> (len shopping-list)
4
elux:18>> (len data)
7
```

ELux support some basic *functional programming* builtins include *map*, *filter*
*reduce*, *zip+, *take*

```elux
elux:19>> (fun square(x) (* x x))
elux:20>> (map square data)
[1 4 9 16 25 36 49]
elux:21>> (var xdata (map square data))
[1 4 9 16 25 36 49]
elux:22>> (filter (lambda (x) (= (% x 2) 0)) xdata)
[4 16 36]
elux:23>> (zip data xdata)
[[1 1] [2 4] [3 9] [4 16] [5 25] [6 36] [7 49]]
```

A complete tutorial will be provided later.

## Planned features

### Arithmetic, bitwise and logical operations

### Data structre and associated methods
- String **work in progress**
- List **work in progress**
- Array **work in progress**
- Set **work in progress**
- Dict **work in progress**

### Module import system

Currently, a special form *import* is supported for import external module, but
the interface will be modified and refined.

### A macro system

A *Common Lisp* like *macro* system is in development.

## Builtin functions
- __Arithmetic functions__: *+*, *-*, *\**, *\%*,
- __Bitwise functions__: *^*, *|*, *&*, *~*,
- __Logical functions__: *and*, *or*, *not*,
- __Other functions__: *println*, *eprintln*, *print*, *eprint*, *panic*,
  *quit*, *exit*, *bye*, *map*, *filter*, *reduce*, *zip*, *enumerate*, *get*, 
  *List.new*, *Set.new*, *Array.new*, *Dict.new*, *len*

