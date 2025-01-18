(module
  (import "fuzzing-support" "log-i32" (func $log (param i32)))

  (func $a (export "a") (result i32)
    (i32.const 10)
  )

  (func $b (export "b") (result i32)
    (i32.const 20)
  )

  (func $c (export "c") (result i32)
    (i32.const 30)
  )

  (func $d (export "d") (result i32)
    (i32.const 40)
  )

  (func $e (export "e") (result i32)
    (i32.const 50)
  )
)

;; Apply JSPI: prepend JSPI = 1 and remove comments around async and await.
;; RUN: echo "JSPI = 1;" > %t.js
;; RUN: node -e "process.stdout.write(require('fs').readFileSync(path.join('%S', '..', '..', '..', 'scripts', 'fuzz_shell.js'), 'utf-8').replace(/[/][*] async [*][/]/g, 'async').replace(/[/][*] await [*][/]/g, 'await'))" >> %t.js

;; Append another run with a random seed, so we reorder and delay execution.
;; RUN: echo "callExports(42);" >> %t.js

;; Run that JS shell with our wasm.
;; RUN: wasm-opt %s -o %t.wasm -q
;; RUN: v8 --wasm-staging %t.js -- %t.wasm | filecheck %s
;;
;; The output here looks a little out of order, in particular because we do not
;; |await| the toplevel callExports() calls. That |await| is only valid if we
;; pass --module, which we do not fuzz with. As a result, the first await
;; operation in the first callExports() leaves that function and continues to
;; the next, but we do get around to executing all the things we need. In
;; particular, the output here should contain two "node result" lines for each
;; of the 5 functions (one from each callExports()). The important thing is that
;; we get a random-like ordering, which includes some defers (each of which has
;; a later finish), showing that we interleave stacks.
;;
;; CHECK: [fuzz-exec] calling a
;; CHECK: [fuzz-exec] calling b
;; CHECK: [fuzz-exec] note result: a => 10
;; CHECK: [fuzz-exec] calling b
;; CHECK: [fuzz-exec] note result: b => 20
;; CHECK: [fuzz-exec] calling a
;; CHECK: (jspi: defer a)
;; CHECK: [fuzz-exec] calling d
;; CHECK: (jspi: defer d)
;; CHECK: [fuzz-exec] calling e
;; CHECK: [fuzz-exec] note result: b => 20
;; CHECK: [fuzz-exec] calling c
;; CHECK: [fuzz-exec] note result: e => 50
;; CHECK: [fuzz-exec] calling c
;; CHECK: (jspi: defer c)
;; CHECK: [fuzz-exec] calling c
;; CHECK: (jspi: finish c)
;; CHECK: [fuzz-exec] note result: c => 30
;; CHECK: [fuzz-exec] calling d
;; CHECK: [fuzz-exec] note result: c => 30
;; CHECK: [fuzz-exec] calling d
;; CHECK: (jspi: finish d)
;; CHECK: [fuzz-exec] note result: d => 40
;; CHECK: [fuzz-exec] calling e
;; CHECK: [fuzz-exec] note result: d => 40
;; CHECK: [fuzz-exec] calling a
;; CHECK: (jspi: finish a)
;; CHECK: [fuzz-exec] note result: a => 10
;; CHECK: [fuzz-exec] note result: e => 50

