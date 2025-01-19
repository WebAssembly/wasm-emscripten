FILE(GLOB interpreter_HEADERS *.h)
set(interpreter_SOURCES
 expression-iterator.cpp
 interpreter.cpp
 ${interpreter_HEADERS}
)
add_library(interpreter OBJECT ${interpreter_SOURCES})
