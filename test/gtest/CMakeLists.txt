include_directories(../../third_party/googletest/googletest/include)
include_directories(../../src/wasm)

set(unittest_SOURCES
  arena.cpp
  source-map.cpp
  cfg.cpp
  dfa_minimization.cpp
  disjoint_sets.cpp
  interpreter.cpp
  json.cpp
  lattices.cpp
  local-graph.cpp
  possible-contents.cpp
  printing.cpp
  scc.cpp
  stringify.cpp
  suffix_tree.cpp
  topological-sort.cpp
  type-builder.cpp
  wat-lexer.cpp
  validator.cpp
)

# suffix_tree.cpp includes LLVM header using std::iterator (deprecated in C++17)
if (NOT MSVC)
  set_source_files_properties(suffix_tree.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

binaryen_add_executable(binaryen-unittests "${unittest_SOURCES}")
target_link_libraries(binaryen-unittests gtest gtest_main)
