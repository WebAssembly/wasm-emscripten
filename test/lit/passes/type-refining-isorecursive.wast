;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --type-refining --closed-world -all -S -o - | filecheck %s

(module
 ;; The types should be refined to a set of three mutually recursive types.

 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $2 (sub (struct (field nullexternref) (field (ref $0)))))

 ;; CHECK:       (type $1 (sub (struct (field nullfuncref) (field (ref $2)))))

 ;; CHECK:       (type $0 (sub (struct (field nullref) (field (ref $1)))))
 (type $0 (sub (struct nullref anyref)))
 (type $1 (sub (struct nullfuncref anyref)))
 (type $2 (sub (struct nullexternref anyref)))

 ;; CHECK:       (type $3 (func (param (ref $0) (ref $1) (ref $2))))

 ;; CHECK:      (func $foo (type $3) (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $0
 ;; CHECK-NEXT:    (ref.null none)
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $1
 ;; CHECK-NEXT:    (ref.null nofunc)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $2
 ;; CHECK-NEXT:    (ref.null noextern)
 ;; CHECK-NEXT:    (local.get $x)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $foo (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
  (drop
   (struct.new $0
    (ref.null none)
    (local.get $y)
   )
  )
  (drop
   (struct.new $1
    (ref.null nofunc)
    (local.get $z)
   )
  )
  (drop
   (struct.new $2
    (ref.null noextern)
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
 ;; CHECK-NEXT:  (type $0 (sub (struct (field (ref null $all)) (field (ref $0)))))

 ;; CHECK:       (type $1 (sub $0 (struct (field (ref null $all)) (field (ref $0)))))

 ;; CHECK:       (type $2 (sub $1 (struct (field (ref null $all)) (field (ref $0)))))

 ;; CHECK:       (type $all (sub (struct (field i32) (field (ref $0)) (field (ref $1)) (field (ref $2)))))
 (type $all (sub (struct i32 anyref anyref anyref)))

 (type $0 (sub (struct anyref anyref)))
 (type $1 (sub $0 (struct anyref anyref)))
 (type $2 (sub $1 (struct anyref anyref)))

 ;; CHECK:       (type $4 (func (param (ref $0) (ref $1) (ref $2))))

 ;; CHECK:      (func $foo (type $4) (param $x (ref $0)) (param $y (ref $1)) (param $z (ref $2))
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
 ;; CHECK-NEXT:    (local.get $all)
 ;; CHECK-NEXT:    (local.get $y)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $1
 ;; CHECK-NEXT:    (local.get $all)
 ;; CHECK-NEXT:    (local.get $z)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (struct.new $2
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
    (local.get $all)
    (local.get $y)
   )
  )
  (drop
   (struct.new $1
    (local.get $all)
    (local.get $z)
   )
  )
  (drop
   (struct.new $2
    (local.get $all)
    (local.get $x)
   )
  )
 )
)
