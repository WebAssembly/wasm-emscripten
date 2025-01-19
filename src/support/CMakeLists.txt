FILE(GLOB support_HEADERS *.h)
set(support_SOURCES
  archive.cpp
  bits.cpp
  colors.cpp
  command-line.cpp
  debug.cpp
  dfa_minimization.cpp
  file.cpp
  intervals.cpp
  istring.cpp
  json.cpp
  name.cpp
  path.cpp
  safe_integer.cpp
  string.cpp
  threads.cpp
  utilities.cpp
  ${support_HEADERS}
)

# The below condition is intended for removal once the suffix_tree and
# suffix_tree_node source files no longer depend on LLVM code in the
# third_party folder
if(EMSCRIPTEN)
  add_library(support OBJECT ${support_SOURCES})
else()
  set(support_with_suffix_tree_SOURCES
    suffix_tree.cpp
    suffix_tree_node.cpp
    ${support_SOURCES}
  )
  add_library(support OBJECT ${support_with_suffix_tree_SOURCES})
endif()
