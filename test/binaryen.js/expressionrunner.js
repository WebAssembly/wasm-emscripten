console.log("// ExpressionRunner.DEFAULT_MAX_DEPTH = " + binaryen.ExpressionRunner.DEFAULT_MAX_DEPTH);

var Flags = binaryen.ExpressionRunner.Flags;
console.log("// ExpressionRunner.Flags.Default = " + Flags.Default);
console.log("// ExpressionRunner.Flags.PreserveSideeffects = " + Flags.PreserveSideeffects);
console.log("// ExpressionRunner.Flags.TraverseCalls = " + Flags.TraverseCalls);

binaryen.setAPITracing(true);

var module = new binaryen.Module();
module.addGlobal("aGlobal", binaryen.i32, true, module.i32.const(0));

// Should evaluate down to a constant
var runner = new binaryen.ExpressionRunner(module);
var expr = runner.runAndDispose(
  module.i32.add(
    module.i32.const(1),
    module.i32.const(2)
  )
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":3}');

// Should traverse control structures
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.i32.add(
    module.i32.const(1),
    module.if(
      module.i32.const(0),
      module.i32.const(0),
      module.i32.const(3)
    )
  ),
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":4}');

// Should be unable to evaluate a local if not explicitly specified
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.i32.add(
    module.local.get(0, binaryen.i32),
    module.i32.const(1)
  )
);
assert(expr === 0);

// Should handle traps properly
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.unreachable()
);
assert(expr === 0);

// Should ignore `local.tee` side-effects if just evaluating the expression
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.i32.add(
    module.local.tee(0, module.i32.const(4), binaryen.i32),
    module.i32.const(1)
  )
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":5}');

// Should preserve any side-effects if explicitly requested
runner = new binaryen.ExpressionRunner(module, Flags.PreserveSideeffects);
expr = runner.runAndDispose(
  module.i32.add(
    module.local.tee(0, module.i32.const(4), binaryen.i32),
    module.i32.const(1)
  )
);
assert(expr === 0);

// Should work with temporary values if just evaluating the expression
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.i32.add(
    module.block(null, [
      module.local.set(0, module.i32.const(2)),
      module.local.get(0, binaryen.i32)
    ], binaryen.i32),
    module.block(null, [
      module.global.set("aGlobal", module.i32.const(4)),
      module.global.get("aGlobal", binaryen.i32)
    ], binaryen.i32)
  )
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":6}');

// Should pick up explicitly preset values
runner = new binaryen.ExpressionRunner(module, Flags.PreserveSideeffects);
assert(runner.setLocalValue(0, module.i32.const(3)));
assert(runner.setGlobalValue("aGlobal", module.i32.const(4)));
expr = runner.runAndDispose(
  module.i32.add(
    module.local.get(0, binaryen.i32),
    module.global.get("aGlobal", binaryen.i32)
  )
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":7}');

// Should traverse into (simple) functions if requested
runner = new binaryen.ExpressionRunner(module, Flags.TraverseCalls);
module.addFunction("add", binaryen.createType([ binaryen.i32, binaryen.i32 ]), binaryen.i32, [],
  module.block(null, [
    module.i32.add(
      module.local.get(0, binaryen.i32),
      module.local.get(1, binaryen.i32)
    )
  ], binaryen.i32)
);
expr = runner.runAndDispose(
  module.i32.add(
    module.i32.const(1),
    module.call("add", [
      module.i32.const(3),
      module.i32.const(4)
    ], binaryen.i32)
  )
);
assert(JSON.stringify(binaryen.getExpressionInfo(expr)) === '{"id":14,"type":2,"value":8}');

// Should not attempt to traverse into functions if not explicitly set
runner = new binaryen.ExpressionRunner(module);
expr = runner.runAndDispose(
  module.i32.add(
    module.i32.const(1),
    module.call("add", [
      module.i32.const(3),
      module.i32.const(4)
    ], binaryen.i32)
  )
);
assert(expr === 0);

binaryen.setAPITracing(false);
