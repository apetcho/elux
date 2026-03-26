set(ELUX_LIBSRC
    elux.hpp
    elux.cpp
    elux-parser.cpp
    elux-core.cpp
    elux-builtins.cpp)

set(ELUX_EXECSRC ${ELUX_LIBSRC} elux-main.cpp)

#add_library(elux STATIC ${ELUX_LIBSRC})
add_executable(ELux ${ELUX_EXECSRC})