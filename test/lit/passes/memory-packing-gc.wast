;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; Test that memory-packing does the right thing in the presence of array.new_data.

;; RUN: foreach %s %t wasm-opt --memory-packing --all-features -S -o - | filecheck %s

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  ;; CHECK:      (data "hello")
  (data "hello")

  ;; CHECK:      (func $array-new-data (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $array-new-data (result (ref $array))
    (array.new_data $array 0
      (i32.const 0)
      (i32.const 5)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  ;; CHECK:      (data "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hello\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
  (data "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00hello\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")

  ;; CHECK:      (func $no-drop-ends (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 5)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-drop-ends (result (ref $array))
    (array.new_data $array 0
      (i32.const 0)
      (i32.const 5)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  ;; CHECK:      (memory $mem 1 1)
  (memory $mem 1 1)

  ;; CHECK:      (data "optimize\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")
  (data "optimize\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")

  ;; CHECK:      (func $no-split (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 8)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-split (result (ref $array))
    (array.new_data $array 0
      (i32.const 0)
      (i32.const 8)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  ;; CHECK:      (memory $mem 1 1)
  (memory $mem 1 1)

  (data (i32.const 0) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00optimize\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")

  ;; CHECK:      (data (i32.const 0) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00optimize\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")

  ;; CHECK:      (func $no-split-active (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 0
  ;; CHECK-NEXT:   (i32.const 16)
  ;; CHECK-NEXT:   (i32.const 8)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $no-split-active (result (ref $array))
    (array.new_data $array 0
      (i32.const 16)
      (i32.const 8)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  (data (i32.const 0) "optimize\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")

  ;; CHECK:      (memory $mem 1 1)

  ;; CHECK:      (data (i32.const 0) "optimize")

  ;; CHECK:      (data (i32.const 24) "me")

  ;; CHECK:      (data "but not\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")
  (data "but not\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")

  (memory $mem 1 1)

  ;; CHECK:      (func $renumber-segment (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 2
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $renumber-segment (result (ref $array))
    (array.new_data $array 1
      (i32.const 0)
      (i32.const 7)
    )
  )
)

(module
  ;; CHECK:      (type $none_=>_ref|$array| (func (result (ref $array))))

  ;; CHECK:      (type $array (array i8))
  (type $array (array i8))

  (data "dead\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00segment")

  ;; CHECK:      (memory $mem 1 1)

  ;; CHECK:      (data "but not\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")
  (data "but not\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00me")

  (memory $mem 1 1)

  ;; CHECK:      (func $renumber-segment (result (ref $array))
  ;; CHECK-NEXT:  (array.new_data $array 0
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (i32.const 7)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $renumber-segment (result (ref $array))
    (array.new_data $array 1
      (i32.const 0)
      (i32.const 7)
    )
  )
)
