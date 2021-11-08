;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --inlining -all -S -o - | filecheck %s

(module
  ;; ---------------------------------------------------------------------------
  ;; CHECK:      (import "a" "b" (func $foo (result i32)))
  (import "a" "b" (func $foo (result i32)))
  ;; CHECK:      (tag $tag$0 (param i32))
  (tag $tag$0 (param i32))
  (func $callee-with-label
    (try $label
      (do)
      (catch $tag$0
        (drop
          (pop i32)
        )
      )
    )
  )

  ;; Properly ensure unique try labels after an inlining

  ;; CHECK:      (func $caller-with-label (param $x i32)
  ;; CHECK-NEXT:  (loop $label
  ;; CHECK-NEXT:   (block
  ;; CHECK-NEXT:    (block $__inlined_func$callee-with-label
  ;; CHECK-NEXT:     (try $label0
  ;; CHECK-NEXT:      (do
  ;; CHECK-NEXT:       (nop)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (catch $tag$0
  ;; CHECK-NEXT:       (drop
  ;; CHECK-NEXT:        (pop i32)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (br_if $label
  ;; CHECK-NEXT:    (call $foo)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller-with-label (param $x i32)
    (loop $label ;; The same label as the try that will be inlined into here
      (call $callee-with-label)
      (br_if $label
        (call $foo)
      )
    )
  )

  ;; ---------------------------------------------------------------------------
  ;; CHECK:      (func $callee-with-try-delegate
  ;; CHECK-NEXT:  (try $label$3
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (delegate 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $callee-with-try-delegate
    (try $label$3
      (do)
      (delegate 0)
    )
  )

  ;; For now, do not inline a try-delegate

  ;; CHECK:      (func $caller
  ;; CHECK-NEXT:  (call $callee-with-try-delegate)
  ;; CHECK-NEXT: )
  (func $caller
    (call $callee-with-try-delegate)
  )

  ;; ---------------------------------------------------------------------------
  (func $callee (result i32)
    (i32.const 42)
  )

  ;; Properly support inlining into a function with a try-delegate

  ;; CHECK:      (func $caller-with-try-delegate (result i32)
  ;; CHECK-NEXT:  (try $label$3
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (nop)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (delegate 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (block (result i32)
  ;; CHECK-NEXT:   (block $__inlined_func$callee (result i32)
  ;; CHECK-NEXT:    (i32.const 42)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $caller-with-try-delegate (result i32)
    (try $label$3
      (do)
      (delegate 0)
    )
    (call $callee)
  )
)
