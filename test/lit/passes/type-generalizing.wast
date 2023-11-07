;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --dce --experimental-type-generalizing -all -S -o - | filecheck %s

(module

 ;; CHECK:      (type $0 (func (result eqref)))

 ;; CHECK:      (type $1 (func))

 ;; CHECK:      (type $2 (func (param anyref)))

 ;; CHECK:      (type $3 (func (param i31ref)))

 ;; CHECK:      (type $4 (func (param anyref eqref)))

 ;; CHECK:      (type $5 (func (param eqref)))

 ;; CHECK:      (func $unconstrained (type $1)
 ;; CHECK-NEXT:  (local $x i32)
 ;; CHECK-NEXT:  (local $y anyref)
 ;; CHECK-NEXT:  (local $z (anyref i32))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $unconstrained
  ;; This non-ref local should be unmodified
  (local $x i32)
  ;; There is no constraint on the type of this local, so make it top.
  (local $y anyref)
  ;; We cannot optimize tuple locals yet, so leave it unchanged.
  (local $z (anyref i32))
 )

 ;; CHECK:      (func $implicit-return (type $0) (result eqref)
 ;; CHECK-NEXT:  (local $var eqref)
 ;; CHECK-NEXT:  (local.get $var)
 ;; CHECK-NEXT: )
 (func $implicit-return (result eqref)
  ;; This will be optimized, but only to eqref because of the constaint from the
  ;; implicit return.
  (local $var i31ref)
  (local.get $var)
 )

 ;; CHECK:      (func $implicit-return-unreachable (type $0) (result eqref)
 ;; CHECK-NEXT:  (local $var anyref)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $implicit-return-unreachable (result eqref)
  ;; Now will optimize this all the way to anyref because we don't analyze
  ;; unreachable code. This would not validate if we didn't run DCE first.
  (local $var i31ref)
  (unreachable)
  (local.get $var)
 )

 ;; CHECK:      (func $if (type $0) (result eqref)
 ;; CHECK-NEXT:  (local $x eqref)
 ;; CHECK-NEXT:  (local $y eqref)
 ;; CHECK-NEXT:  (if (result eqref)
 ;; CHECK-NEXT:   (i32.const 0)
 ;; CHECK-NEXT:   (local.get $x)
 ;; CHECK-NEXT:   (local.get $y)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $if (result (eqref))
  (local $x i31ref)
  (local $y i31ref)
  (if (result i31ref)
   (i32.const 0)
   ;; Require that typeof($x) <: eqref.
   (local.get $x)
   ;; Require that typeof($y) <: eqref.
   (local.get $y)
  )
 )

 ;; CHECK:      (func $local-set (type $1)
 ;; CHECK-NEXT:  (local $var anyref)
 ;; CHECK-NEXT:  (local.set $var
 ;; CHECK-NEXT:   (ref.i31
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $local-set
  ;; This will be optimized to anyref.
  (local $var i31ref)
  ;; Require that (ref i31) <: typeof($var).
  (local.set $var
   (i31.new
    (i32.const 0)
   )
  )
 )

 ;; CHECK:      (func $local-get-set (type $2) (param $dest anyref)
 ;; CHECK-NEXT:  (local $var anyref)
 ;; CHECK-NEXT:  (local.set $dest
 ;; CHECK-NEXT:   (local.get $var)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $local-get-set (param $dest anyref)
  ;; This will be optimized to anyref.
  (local $var i31ref)
  ;; Require that typeof($var) <: typeof($dest).
  (local.set $dest
   (local.get $var)
  )
 )

 ;; CHECK:      (func $local-get-set-unreachable (type $3) (param $dest i31ref)
 ;; CHECK-NEXT:  (local $var anyref)
 ;; CHECK-NEXT:  (unreachable)
 ;; CHECK-NEXT: )
 (func $local-get-set-unreachable (param $dest i31ref)
  ;; This is not constrained by reachable code, so we will optimize it.
  (local $var i31ref)
  (unreachable)
  ;; This would require that typeof($var) <: typeof($dest), except it is
  ;; unreachable. This would not validate if we didn't run DCE first.
  (local.set $dest
   (local.tee $var
    (local.get $var)
   )
  )
 )

 ;; CHECK:      (func $local-get-set-join (type $4) (param $dest1 anyref) (param $dest2 eqref)
 ;; CHECK-NEXT:  (local $var eqref)
 ;; CHECK-NEXT:  (local.set $dest1
 ;; CHECK-NEXT:   (local.get $var)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $dest2
 ;; CHECK-NEXT:   (local.get $var)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $local-get-set-join (param $dest1 anyref) (param $dest2 eqref)
  ;; This wll be optimized to eqref.
  (local $var i31ref)
  ;; Require that typeof($var) <: typeof($dest1).
  (local.set $dest1
   (local.get $var)
  )
  ;; Also require that typeof($var) <: typeof($dest2).
  (local.set $dest2
   (local.get $var)
  )
 )

 ;; CHECK:      (func $local-get-set-chain (type $0) (result eqref)
 ;; CHECK-NEXT:  (local $a eqref)
 ;; CHECK-NEXT:  (local $b eqref)
 ;; CHECK-NEXT:  (local $c eqref)
 ;; CHECK-NEXT:  (local.set $b
 ;; CHECK-NEXT:   (local.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $c
 ;; CHECK-NEXT:   (local.get $b)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.get $c)
 ;; CHECK-NEXT: )
 (func $local-get-set-chain (result eqref)
  (local $a i31ref)
  (local $b i31ref)
  (local $c i31ref)
  ;; Require that typeof($a) <: typeof($b).
  (local.set $b
   (local.get $a)
  )
  ;; Require that typeof($b) <: typeof($c).
  (local.set $c
   (local.get $b)
  )
  ;; Require that typeof($c) <: eqref.
  (local.get $c)
 )

 ;; CHECK:      (func $local-get-set-chain-out-of-order (type $0) (result eqref)
 ;; CHECK-NEXT:  (local $a eqref)
 ;; CHECK-NEXT:  (local $b eqref)
 ;; CHECK-NEXT:  (local $c eqref)
 ;; CHECK-NEXT:  (local.set $c
 ;; CHECK-NEXT:   (local.get $b)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.set $b
 ;; CHECK-NEXT:   (local.get $a)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (local.get $c)
 ;; CHECK-NEXT: )
 (func $local-get-set-chain-out-of-order (result eqref)
  (local $a i31ref)
  (local $b i31ref)
  (local $c i31ref)
  ;; Require that typeof($b) <: typeof($c).
  (local.set $c
   (local.get $b)
  )
  ;; Require that typeof($a) <: typeof($b). We don't know until we evaluate the
  ;; set above that this will constrain $a to eqref.
  (local.set $b
   (local.get $a)
  )
  ;; Require that typeof($c) <: eqref.
  (local.get $c)
 )

 ;; CHECK:      (func $local-tee (type $5) (param $dest eqref)
 ;; CHECK-NEXT:  (local $var eqref)
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (local.tee $dest
 ;; CHECK-NEXT:    (local.tee $var
 ;; CHECK-NEXT:     (ref.i31
 ;; CHECK-NEXT:      (i32.const 0)
 ;; CHECK-NEXT:     )
 ;; CHECK-NEXT:    )
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $local-tee (param $dest eqref)
  ;; This will be optimized to eqref.
  (local $var i31ref)
  (drop
   (local.tee $dest
    (local.tee $var
     (i31.new
      (i32.const 0)
     )
    )
   )
  )
 )

 ;; CHECK:      (func $i31-get (type $1)
 ;; CHECK-NEXT:  (local $nullable i31ref)
 ;; CHECK-NEXT:  (local $nonnullable (ref i31))
 ;; CHECK-NEXT:  (local.set $nonnullable
 ;; CHECK-NEXT:   (ref.i31
 ;; CHECK-NEXT:    (i32.const 0)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (i31.get_s
 ;; CHECK-NEXT:    (local.get $nullable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT:  (drop
 ;; CHECK-NEXT:   (i31.get_u
 ;; CHECK-NEXT:    (local.get $nonnullable)
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $i31-get
  ;; This must stay an i31ref.
  (local $nullable i31ref)
  ;; This one could be relaxed to be nullable in principle, but we keep it non-nullable.
  (local $nonnullable (ref i31))
  ;; Initialize the non-nullable local for validation purposes.
  (local.set $nonnullable
   (i31.new
    (i32.const 0)
   )
  )
  (drop
   ;; Require that typeof($nullable) <: i31ref.
   (i31.get_s
    (local.get $nullable)
   )
  )
  (drop
   ;; Require that typeof($nonnullable) <: i31ref.
   (i31.get_u
    (local.get $nonnullable)
   )
  )
 )
)
