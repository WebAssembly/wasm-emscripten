FILE(GLOB ir_HEADERS *.h)
set(ir_SOURCES
  ExpressionAnalyzer.cpp
  ExpressionManipulator.cpp
  debuginfo.cpp
  drop.cpp
  effects.cpp
  eh-utils.cpp
  export-utils.cpp
  global-utils.cpp
  intrinsics.cpp
  lubs.cpp
  memory-utils.cpp
  module-utils.cpp
  names.cpp
  possible-contents.cpp
  properties.cpp
  LocalGraph.cpp
  LocalStructuralDominance.cpp
  ReFinalize.cpp
  return-utils.cpp
  stack-utils.cpp
  table-utils.cpp
  type-updating.cpp
  module-splitting.cpp
  ${ir_HEADERS}
)
add_library(ir OBJECT ${ir_SOURCES})
