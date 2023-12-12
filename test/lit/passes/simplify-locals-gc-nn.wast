;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --simplify-locals -all -S -o - | filecheck %s

(module
  ;; CHECK:      (func $test-nn (type $0)
  ;; CHECK-NEXT:  (local $nn anyref)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (local.set $nn
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null none)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch_all
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (local.get $nn)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-nn
    (local $nn (ref any))
    ;; We can sink this set into the try, but the spec does not allow it to
    ;; remain non-nullable. Even though we are not changing dominance (we are
    ;; not changing it, because there is nothing that can throw in the try's
    ;; body that can reach the catch_all before the local.set that we move
    ;; there). See
    ;; https://github.com/WebAssembly/function-references/issues/44#issuecomment-1083146887
    (local.set $nn
      (ref.as_non_null
        (ref.null any)
      )
    )
    (try
      (do
        (drop
          (local.get $nn)
        )
      )
      (catch_all
        (drop
          (local.get $nn)
        )
      )
    )
  )

  ;; CHECK:      (func $test-nn-tuple (type $0)
  ;; CHECK-NEXT:  (local $nn (i32 anyref i64))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (local.set $nn
  ;; CHECK-NEXT:     (tuple.make 3
  ;; CHECK-NEXT:      (i32.const 0)
  ;; CHECK-NEXT:      (ref.as_non_null
  ;; CHECK-NEXT:       (ref.null none)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (i64.const 0)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch_all
  ;; CHECK-NEXT:    (tuple.drop 3
  ;; CHECK-NEXT:     (tuple.make 3
  ;; CHECK-NEXT:      (tuple.extract 0
  ;; CHECK-NEXT:       (local.get $nn)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (ref.as_non_null
  ;; CHECK-NEXT:       (tuple.extract 1
  ;; CHECK-NEXT:        (local.get $nn)
  ;; CHECK-NEXT:       )
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:      (tuple.extract 2
  ;; CHECK-NEXT:       (local.get $nn)
  ;; CHECK-NEXT:      )
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-nn-tuple
    ;; Same as above, but now the local is a tuple containing a non-nullable element
    (local $nn (i32 (ref any) i64))
    (local.set $nn
      (tuple.make 3
        (i32.const 0)
        (ref.as_non_null
          (ref.null any)
        )
        (i64.const 0)
      )
    )
    (try
      (do
        (tuple.drop 3
          (local.get $nn)
        )
      )
      (catch_all
        (tuple.drop 3
          (local.get $nn)
        )
      )
    )
  )

  ;; CHECK:      (func $test-nullable (type $0)
  ;; CHECK-NEXT:  (local $nullable anyref)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT:  (try $try
  ;; CHECK-NEXT:   (do
  ;; CHECK-NEXT:    (local.set $nullable
  ;; CHECK-NEXT:     (ref.as_non_null
  ;; CHECK-NEXT:      (ref.null none)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (catch_all
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (local.get $nullable)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test-nullable
    ;; As above, but now the local is nullable. Here we can optimize the set
    ;; into the try, with no other necessary changes.
    (local $nullable (ref null any))
    (local.set $nullable
      (ref.as_non_null
        (ref.null any)
      )
    )
    (try
      (do
        (drop
          (local.get $nullable)
        )
      )
      (catch_all
        (drop
          (local.get $nullable)
        )
      )
    )
  )

  ;; CHECK:      (func $if-return-tuple-nn (type $0)
  ;; CHECK-NEXT:  (local $temp ((ref func) nullref))
  ;; CHECK-NEXT:  (if
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (nop)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $if-return-tuple-nn
    (local $temp ((ref func) (ref null none)))
    ;; We should not emit a return value for this if, as the tuple has a non-
    ;; nullable element, so it is nondefaultable.
    ;;
    ;; Instead, we can remove the local.set entirely, as it has no gets.
    (if
      (i32.const 0)
      (local.set $temp
        (tuple.make 2
          (ref.func $if-return-tuple-nn)
          (ref.null none)
        )
      )
    )
  )
)
