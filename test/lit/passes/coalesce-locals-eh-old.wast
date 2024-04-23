;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --coalesce-locals -all -S -o - | filecheck %s

(module
  ;; CHECK:      (tag $e)

  ;; CHECK:      (tag $any (param (ref any)))

  ;; CHECK:      (func $bar (type $2) (result i32)
  ;; CHECK-NEXT:  (i32.const 1984)
  ;; CHECK-NEXT: )
  (func $bar (result i32)
    (i32.const 1984)
  )

 (tag $e)

 (tag $any (param (ref any)))

  ;; CHECK:      (func $bug-cfg-traversal (type $3) (param $0 i32) (result i32)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (local.set $0
  ;; CHECK-NEXT:     (call $bar)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch_all
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (local.get $0)
  ;; CHECK-NEXT: )
  (func $bug-cfg-traversal (param $0 i32) (result i32)
    (local $x i32)
    ;; This is a regrssion test case for a bug in cfg-traversal for EH.
    ;; See https://github.com/WebAssembly/binaryen/pull/3594
    (try
      (do
        (local.set $x
          ;; the call may or may not throw, so we may reach the get of $x
          (call $bar)
        )
      )
      (catch_all
        (unreachable)
      )
    )
    (local.get $x)
  )

  ;; CHECK:      (func $0 (type $0)
  ;; CHECK-NEXT:  (local $0 anyref)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch $any
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (pop (ref any))
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $0
    (local $0 (ref null any))
    (try
      (do)
      (catch $any
        (drop
          ;; There is a difference between the type of the value here and the
          ;; type of the local, due to the local being nullable. We should not
          ;; error on that as we replace the tee with a drop (as it has no
          ;; gets).
          (local.tee $0
            (pop (ref any))
          )
        )
      )
    )
  )
)
