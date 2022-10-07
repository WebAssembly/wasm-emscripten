;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --hybrid --type-refining -all -S -o - | filecheck %s

(module
 ;; The types should be refined to a set of three mutually recursive types.

 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $0 (struct_subtype (field anyref) (field (ref $1)) data))
 (type $0 (struct_subtype (ref null any) anyref data))
 ;; CHECK:       (type $1 (struct_subtype (field eqref) (field (ref $2)) data))
 (type $1 (struct_subtype (ref null eq) anyref data))
 ;; CHECK:       (type $2 (struct_subtype (field i31ref) (field (ref $0)) data))
 (type $2 (struct_subtype (ref null i31) anyref data))

 ;; CHECK:       (type $ref|$0|_ref|$1|_ref|$2|_=>_none (func_subtype (param (ref $0) (ref $1) (ref $2)) func))

 ;; CHECK:      (func $foo (type $ref|$0|_ref|$1|_ref|$2|_=>_none) (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $0
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $1
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $2
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
  (drop
   (struct.new $0
    (ref.null any)
    (local.get $y)
   )
  )
  (drop
   (struct.new $1
    (ref.null eq)
    (local.get $z)
   )
  )
  (drop
   (struct.new $2
    (ref.null i31)
    (local.get $x)
   )
  )
 )
)

(module
 ;; The types will all be mutually recursive because they all reference and are
 ;; referenced by $all, but now we need to worry about ordering supertypes
 ;; correctly.

 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $all (struct_subtype (field i32) (field (ref $0)) (field (ref $1)) (field (ref $2)) data))
 (type $all (struct_subtype i32 anyref anyref anyref data))

 ;; CHECK:       (type $0 (struct_subtype (field anyref) (field (ref null $all)) (field (ref $0)) data))
 (type $0 (struct_subtype (ref null any) anyref anyref data))
 ;; CHECK:       (type $1 (struct_subtype (field eqref) (field (ref null $all)) (field (ref $0)) $0))
 (type $1 (struct_subtype (ref null eq) anyref anyref $0))
 ;; CHECK:       (type $2 (struct_subtype (field i31ref) (field (ref null $all)) (field (ref $0)) $1))
 (type $2 (struct_subtype (ref null i31) anyref anyref $1))

 ;; CHECK:       (type $ref|$0|_ref|$1|_ref|$2|_=>_none (func_subtype (param (ref $0) (ref $1) (ref $2)) func))

 ;; CHECK:      (func $foo (type $ref|$0|_ref|$1|_ref|$2|_=>_none) (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
 ;; CHECK-NEXT:  (local $all (ref null $all))
 ;; CHECK-NEXT:  (local.set $all
 ;; CHECK-NEXT:   (struct.new $all
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $0
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $all)
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $1
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $all)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $2
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $all)
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
  (local $all (ref null $all))
  (local.set $all
   (struct.new $all
    (i32.const 0)
    (local.get $x)
    (local.get $y)
    (local.get $z)
   )
  )
  (drop
   (struct.new $0
    (ref.null any)
    (local.get $all)
    (local.get $y)
   )
  )
  (drop
   (struct.new $1
    (ref.null eq)
    (local.get $all)
    (local.get $z)
   )
  )
  (drop
   (struct.new $2
    (ref.null i31)
    (local.get $all)
    (local.get $x)
   )
  )
 )
)
