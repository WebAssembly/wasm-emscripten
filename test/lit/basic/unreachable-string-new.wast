;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt -all %s -S -o - | filecheck %s

(module
  ;; CHECK:      (type $array16 (array i16))

  ;; CHECK:      (type $array8 (array i8))
  (type $array8 (array i8))
  (type $array16 (array i16))

  ;; CHECK:      (func $i8-bad-array (type $0) (result (ref string))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref array))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (string.new_lossy_utf8_array
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i8-bad-array (result (ref string))
    block (result (ref array))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_lossy_utf8_array
  )

  ;; CHECK:      (func $i8-bad-array16 (type $0) (result (ref string))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $array16))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (string.new_lossy_utf8_array
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i8-bad-array16 (result (ref string))
    block (result (ref $array16))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_lossy_utf8_array
  )

  ;; CHECK:      (func $i8-ok (type $0) (result (ref string))
  ;; CHECK-NEXT:  (string.new_lossy_utf8_array
  ;; CHECK-NEXT:   (block (result (ref $array8))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i8-ok (result (ref string))
    block (result (ref $array8))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_lossy_utf8_array
  )

  ;; CHECK:      (func $i16-bad-array (type $0) (result (ref string))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref array))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (string.new_wtf16_array
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i16-bad-array (result (ref string))
    block (result (ref array))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_wtf16_array
  )

  ;; CHECK:      (func $i16-bad-array8 (type $0) (result (ref string))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (block (result (ref $array8))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (string.new_wtf16_array
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i16-bad-array8 (result (ref string))
    block (result (ref $array8))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_wtf16_array
  )

  ;; CHECK:      (func $i16-ok (type $0) (result (ref string))
  ;; CHECK-NEXT:  (string.new_wtf16_array
  ;; CHECK-NEXT:   (block (result (ref $array16))
  ;; CHECK-NEXT:    (unreachable)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $i16-ok (result (ref string))
    block (result (ref $array16))
      unreachable
    end
    i32.const 0
    unreachable
    string.new_wtf16_array
  )
)
