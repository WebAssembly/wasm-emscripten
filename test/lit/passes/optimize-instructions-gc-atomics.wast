;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt %s -all --optimize-instructions -S -o - | filecheck %s

(module
  ;; CHECK:      (type $unshared (struct (field (mut i32))))

  ;; CHECK:      (type $shared (shared (struct (field (mut i32)))))
  (type $shared (shared (struct (field (mut i32)))))
  (type $unshared (struct (field (mut i32))))

  ;; CHECK:      (func $get-unordered-unshared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.get $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-unordered-unshared (result i32)
    (struct.get $unshared 0
      (struct.new_default $unshared)
    )
  )

  ;; CHECK:      (func $get-unordered-shared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.get $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-unordered-shared (result i32)
    (struct.get $shared 0
      (struct.new_default $shared)
    )
  )

  ;; CHECK:      (func $get-seqcst-unshared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.atomic.get $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-seqcst-unshared (result i32)
    (struct.atomic.get seqcst $unshared 0
      (struct.new_default $unshared)
    )
  )

  ;; CHECK:      (func $get-seqcst-shared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.atomic.get $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-seqcst-shared (result i32)
    (struct.atomic.get seqcst $shared 0
      (struct.new_default $shared)
    )
  )

  ;; CHECK:      (func $get-acqrel-unshared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.get $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-acqrel-unshared (result i32)
    ;; This can be relaxed to unordered
    (struct.atomic.get acqrel $unshared 0
      (struct.new_default $unshared)
    )
  )

  ;; CHECK:      (func $get-acqrel-shared (type $2) (result i32)
  ;; CHECK-NEXT:  (struct.atomic.get acqrel $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $get-acqrel-shared (result i32)
    (struct.atomic.get acqrel $shared 0
      (struct.new_default $shared)
    )
  )

  ;; CHECK:      (func $set-unordered-unshared (type $3)
  ;; CHECK-NEXT:  (struct.set $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-unordered-unshared
    (struct.set $unshared 0
      (struct.new_default $unshared)
      (i32.const 0)
    )
  )

  ;; CHECK:      (func $set-unordered-shared (type $3)
  ;; CHECK-NEXT:  (struct.set $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-unordered-shared
    (struct.set $shared 0
      (struct.new_default $shared)
      (i32.const 0)
    )
  )

  ;; CHECK:      (func $set-seqcst-unshared (type $3)
  ;; CHECK-NEXT:  (struct.atomic.set $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-seqcst-unshared
    (struct.atomic.set seqcst $unshared 0
      (struct.new_default $unshared)
      (i32.const 0)
    )
  )

  ;; CHECK:      (func $set-seqcst-shared (type $3)
  ;; CHECK-NEXT:  (struct.atomic.set $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-seqcst-shared
    (struct.atomic.set seqcst $shared 0
      (struct.new_default $shared)
      (i32.const 0)
    )
  )

  ;; CHECK:      (func $set-acqrel-unshared (type $3)
  ;; CHECK-NEXT:  (struct.set $unshared 0
  ;; CHECK-NEXT:   (struct.new_default $unshared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-acqrel-unshared
    ;; This can be relaxed to unordered.
    (struct.atomic.set acqrel $unshared 0
      (struct.new_default $unshared)
      (i32.const 0)
    )
  )

  ;; CHECK:      (func $set-acqrel-shared (type $3)
  ;; CHECK-NEXT:  (struct.atomic.set acqrel $shared 0
  ;; CHECK-NEXT:   (struct.new_default $shared)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $set-acqrel-shared
    (struct.atomic.set acqrel $shared 0
      (struct.new_default $shared)
      (i32.const 0)
    )
  )
)