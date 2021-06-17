console.log("SideEffects.None=" + binaryen.SideEffects.None);
console.log("SideEffects.Branches=" + binaryen.SideEffects.Branches);
console.log("SideEffects.Calls=" + binaryen.SideEffects.Calls);
console.log("SideEffects.ReadsLocal=" + binaryen.SideEffects.ReadsLocal);
console.log("SideEffects.WritesLocal=" + binaryen.SideEffects.WritesLocal);
console.log("SideEffects.ReadsGlobal=" + binaryen.SideEffects.ReadsGlobal);
console.log("SideEffects.WritesGlobal=" + binaryen.SideEffects.WritesGlobal);
console.log("SideEffects.ReadsMemory=" + binaryen.SideEffects.ReadsMemory);
console.log("SideEffects.WritesMemory=" + binaryen.SideEffects.WritesMemory);
console.log("SideEffects.ImplicitTrap=" + binaryen.SideEffects.ImplicitTrap);
console.log("SideEffects.IsAtomic=" + binaryen.SideEffects.IsAtomic);
console.log("SideEffects.Throws=" + binaryen.SideEffects.Throws);
console.log("SideEffects.DanglingPop=" + binaryen.SideEffects.DanglingPop);
console.log("SideEffects.ReadsTable=" + binaryen.SideEffects.ReadsTable);
console.log("SideEffects.WritesTable=" + binaryen.SideEffects.WritesTable);
console.log("SideEffects.ReadsElementSegment=" + binaryen.SideEffects.ReadsElementSegment);
console.log("SideEffects.DropsElementSegment=" + binaryen.SideEffects.DropsElementSegment);
console.log("SideEffects.Any=" + binaryen.SideEffects.Any);

var module = new binaryen.Module();
assert(
  binaryen.getSideEffects(
    module.i32.const(1)
  )
  ==
  binaryen.SideEffects.None
);
assert(
  binaryen.getSideEffects(
    module.br("test")
  )
  ==
  binaryen.SideEffects.Branches
);
assert(
  binaryen.getSideEffects(
    module.call("test", [], binaryen.i32)
  )
  ==
  binaryen.SideEffects.Calls
);
assert(
  binaryen.getSideEffects(
    module.local.get("test", binaryen.i32)
  )
  ==
  binaryen.SideEffects.ReadsLocal
);
assert(
  binaryen.getSideEffects(
    module.local.set("test",
      module.i32.const(1)
    )
  )
  ==
  binaryen.SideEffects.WritesLocal
);
assert(
  binaryen.getSideEffects(
    module.global.get("test", binaryen.i32)
  )
  ==
  binaryen.SideEffects.ReadsGlobal
);
assert(
  binaryen.getSideEffects(
    module.global.set("test", module.i32.const(1))
  )
  ==
  binaryen.SideEffects.WritesGlobal
);
assert(
  binaryen.getSideEffects(
    module.table.get("test", module.i32.const(0), binaryen.funcref)
  )
  ==
  binaryen.SideEffects.ReadsTable | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.table.set("test", module.ref.func("f", binaryen.funcref), module.i32.const(1))
  )
  ==
  binaryen.SideEffects.WritesTable | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.table.init("t1", "e1", module.i32.const(10), module.i32.const(0), module.i32.const(0))
  )
  ==
  binaryen.SideEffects.ReadsElementSegment | binaryen.SideEffects.WritesTable | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.elem.drop("test")
  )
  ==
  binaryen.SideEffects.DropsElementSegment | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.i32.load(0, 0,
      module.i32.const(0)
    )
  )
  ==
  binaryen.SideEffects.ReadsMemory | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.i32.store(0, 0,
      module.i32.const(0),
      module.i32.const(1)
    )
  )
  ==
  binaryen.SideEffects.WritesMemory | binaryen.SideEffects.ImplicitTrap
);
assert(
  binaryen.getSideEffects(
    module.i32.div_s(
      module.i32.const(1),
      module.i32.const(0)
    )
  )
  ==
  binaryen.SideEffects.ImplicitTrap
);

// If exception handling feature is enabled, calls can throw
module.setFeatures(binaryen.Features.All);
assert(
  binaryen.getSideEffects(
    module.call("test", [], binaryen.i32),
    module.getFeatures()
  )
  ==
  binaryen.SideEffects.Calls | binaryen.SideEffects.Throws
);

assert(
  binaryen.getSideEffects(
    module.drop(module.i32.pop()),
    module.getFeatures()
  )
  ==
  binaryen.SideEffects.DanglingPop
);
