;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.
;; RUN: foreach %s %t wasm-opt --closed-world --type-merging --remove-unused-types -all -S -o - | filecheck %s

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct (field anyref))))
    (type $A (sub (struct (field anyref))))
    (type $B (sub $A (struct (field anyref))))
    ;; CHECK:       (type $G (sub final $A (struct (field anyref))))

    ;; CHECK:       (type $F (sub $A (struct (field anyref))))

    ;; CHECK:       (type $E (sub $A (struct (field eqref))))

    ;; CHECK:       (type $D (sub $A (struct (field (ref any)))))

    ;; CHECK:       (type $C (sub $A (struct (field anyref) (field f64))))
    (type $C (sub $A (struct (field anyref) (field f64))))
    (type $D (sub $A (struct (field (ref any)))))
    (type $E (sub $A (struct (field eqref))))
    (type $F (sub $A (struct (field anyref))))
    (type $G (sub final $A (struct (field anyref))))
  )

  ;; CHECK:       (type $6 (func))

  ;; CHECK:      (func $foo (type $6)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (local $d (ref null $D))
  ;; CHECK-NEXT:  (local $e (ref null $E))
  ;; CHECK-NEXT:  (local $f (ref null $F))
  ;; CHECK-NEXT:  (local $g (ref null $G))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast (ref null $A)
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast (ref null $F)
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo
    ;; $A will remain the same.
    (local $a (ref null $A))
    ;; $B can be merged into $A.
    (local $b (ref null $B))
    ;; $C cannot because it adds a field.
    (local $c (ref null $C))
    ;; $D cannot because it refines a field's nullability.
    (local $d (ref null $D))
    ;; $E cannot because it refines a field's heap type.
    (local $e (ref null $E))
    ;; $F cannot because it has a cast.
    (local $f (ref null $F))
    ;; $G cannot because it changes finality.
    (local $g (ref null $G))

    ;; A cast of $A has no effect.
    (drop
      (ref.cast (ref null $A)
        (local.get $a)
      )
    )
    ;; A cast of $F prevents it from being merged.
    (drop
      (ref.cast (ref null $F)
        (local.get $a)
      )
    )
  )
)

;; Multiple levels of merging.
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct (field i32))))
  (type $A (sub (struct (field i32))))
  (type $B (sub $A (struct (field i32))))
  (type $C (sub $B (struct (field i32))))
  ;; CHECK:       (type $D (sub $A (struct (field i32) (field f64))))
  (type $D (sub $A (struct (field i32) (field f64))))
  (type $E (sub $D (struct (field i32) (field f64))))
  (type $F (sub $E (struct (field i32) (field f64))))
  (type $G (sub $F (struct (field i32) (field f64))))

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $c (ref null $A))
  ;; CHECK-NEXT:  (local $d (ref null $D))
  ;; CHECK-NEXT:  (local $e (ref null $D))
  ;; CHECK-NEXT:  (local $f (ref null $D))
  ;; CHECK-NEXT:  (local $g (ref null $D))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    (local $a (ref null $A))
    ;; $B can be merged into $A.
    (local $b (ref null $B))
    ;; $C can be merged into $B, so it will merge into $A.
    (local $c (ref null $C))
    ;; $D cannot be merged into $A as it adds a field.
    (local $d (ref null $D))
    ;; $E can be merged into $D.
    (local $e (ref null $E))
    ;; $F can be merged into $E, so it will merge into $D.
    (local $f (ref null $F))
    ;; $G can be merged into $F, so it will merge into $D.
    (local $g (ref null $G))
  )
)

;; As above but now $D is a subtype of $C, so there is a single subtype chain
;; in which we have two "merge points" that things get merged into. The results
;; should remain the same as before, everything merged into either $A or $D.
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct (field i32))))
  (type $A (sub (struct (field i32))))
  (type $B (sub $A (struct (field i32))))
  (type $C (sub $B (struct (field i32))))
  ;; CHECK:       (type $D (sub $A (struct (field i32) (field f64))))
  (type $D (sub $C (struct (field i32) (field f64)))) ;; this line changed
  (type $E (sub $D (struct (field i32) (field f64))))
  (type $F (sub $E (struct (field i32) (field f64))))
  (type $G (sub $F (struct (field i32) (field f64))))

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $c (ref null $A))
  ;; CHECK-NEXT:  (local $d (ref null $D))
  ;; CHECK-NEXT:  (local $e (ref null $D))
  ;; CHECK-NEXT:  (local $f (ref null $D))
  ;; CHECK-NEXT:  (local $g (ref null $D))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $c (ref null $C))
    (local $d (ref null $D))
    (local $e (ref null $E))
    (local $f (ref null $F))
    (local $g (ref null $G))
  )
)

(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $X (sub (struct)))
  (type $X (sub (struct)))
  (type $Y (sub $X (struct)))
  ;; CHECK:       (type $A (sub (struct (field (ref null $X)))))
  (type $A (sub (struct (field (ref null $X)))))
  (type $B (sub $A (struct (field (ref null $Y)))))
  ;; CHECK:       (type $C (sub $A (struct (field (ref $X)))))
  (type $C (sub $A (struct (field (ref $Y)))))

  ;; CHECK:       (type $3 (func))

  ;; CHECK:      (func $foo (type $3)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; B can be merged into A even though it refines A's field because that
    ;; refinement will no longer happen after X and Y are also merged.
    (local $a (ref null $A))
    (local $b (ref null $B))
    ;; C cannot be merged because it refines the field's nullability.
    (local $c (ref null $C))
  )
)

(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct (field (ref null $A)))))
  (type $A (sub (struct    (ref null $A))))
  (type $B (sub $A (struct (ref null $B))))

  ;; CHECK:       (type $1 (func))

  ;; CHECK:      (func $foo (type $1)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; A recursive subtype can be merged even though its field is a refinement
    ;; as well.
    (local $a (ref null $A))
    (local $b (ref null $B))
  )
)

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $X (sub (struct (field (ref null $A)) (field f32))))

    ;; CHECK:       (type $A (sub (struct (field (ref null $X)) (field i32))))
    (type $A (sub (struct    (ref null $X) i32)))
    (type $B (sub $A (struct (ref null $Y) i32)))
    (type $X (sub (struct    (ref null $A) f32)))
    (type $Y (sub $X (struct (ref null $B) f32)))
  )

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $x (ref null $X))
  ;; CHECK-NEXT:  (local $y (ref null $X))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; Two mutually referential chains, A->B and X->Y, can be merged into a pair
    ;; of mutually referential types.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $x (ref null $X))
    (local $y (ref null $Y))
  )
)

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct (field (ref null $A)))))
    (type $A (sub (struct    (ref null $X))))
    (type $B (sub $A (struct (ref null $Y))))
    (type $X (sub (struct    (ref null $A))))
    (type $Y (sub $X (struct (ref null $B))))
  )

  ;; CHECK:       (type $1 (func))

  ;; CHECK:      (func $foo (type $1)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $x (ref null $A))
  ;; CHECK-NEXT:  (local $y (ref null $A))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; As above, but now the A->B and X->Y chains are not differentiated by the
    ;; i32 and f32, so all four types can be merged into a single type.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $x (ref null $X))
    (local $y (ref null $Y))
  )
)

(module
  (rec
    (type $A (struct (ref null $X) i32))
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $Y (struct (field (ref null $B)) (field f32)))

    ;; CHECK:       (type $B (struct (field (ref null $Y)) (field i32)))
    (type $B (struct (ref null $Y) i32))
    (type $X (struct (ref null $A) f32))
    (type $Y (struct (ref null $B) f32))
  )
  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $B))
  ;; CHECK-NEXT:  (local $b (ref null $B))
  ;; CHECK-NEXT:  (local $x (ref null $Y))
  ;; CHECK-NEXT:  (local $y (ref null $Y))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; As above with the differentiated chains, but now the types are top-level
    ;; siblings instead of subtypes
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $x (ref null $X))
    (local $y (ref null $Y))
  )
)

(module
  (rec
    (type $A (struct (ref null $X)))
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $B (struct (field (ref null $B))))
    (type $B (struct (ref null $Y)))
    (type $X (struct (ref null $A)))
    (type $Y (struct (ref null $B)))
  )
  ;; CHECK:       (type $1 (func))

  ;; CHECK:      (func $foo (type $1)
  ;; CHECK-NEXT:  (local $a (ref null $B))
  ;; CHECK-NEXT:  (local $b (ref null $B))
  ;; CHECK-NEXT:  (local $x (ref null $B))
  ;; CHECK-NEXT:  (local $y (ref null $B))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; As above, but with all the types merging into a single type.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $x (ref null $X))
    (local $y (ref null $Y))
  )
)

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $X (sub (struct (field (ref null $A)))))

    ;; CHECK:       (type $A (sub (struct (field (ref null $X)))))
    (type $A (sub (struct    (ref null $X))))
    (type $B (sub $A (struct (ref null $Y))))
    (type $X (sub (struct    (ref null $A))))
    (type $Y (sub $X (struct (ref null $B))))
  )

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $x (ref null $X))
  ;; CHECK-NEXT:  (local $y (ref null $X))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.cast (ref $A)
  ;; CHECK-NEXT:    (local.get $a)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $foo
    ;; As above, but now there is a cast to A that prevents A and X from being
    ;; merged.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $x (ref null $X))
    (local $y (ref null $Y))

    (drop
      (ref.cast (ref $A)
        (local.get $a)
      )
    )
  )
)

(module
  ;; Check that a diversity of root types are merged correctly.
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $M (struct (field i32) (field i32)))

  ;; CHECK:       (type $L (struct (field i32)))

  ;; CHECK:       (type $K (func (param i32 i32 i32) (result i32 i32)))

  ;; CHECK:       (type $J (func (param i32 i32) (result i32 i32 i32)))

  ;; CHECK:       (type $I (array (ref $A)))

  ;; CHECK:       (type $H (array (ref null $A)))

  ;; CHECK:       (type $G (array (ref any)))

  ;; CHECK:       (type $F (array anyref))

  ;; CHECK:       (type $E (array i64))

  ;; CHECK:       (type $D (array i32))

  ;; CHECK:       (type $C (array i16))

  ;; CHECK:       (type $B (array (mut i32)))

  ;; CHECK:       (type $A (array i8))
  (type $A  (array i8))
  (type $A' (array i8))
  (type $C  (array i16))
  (type $C' (array i16))
  (type $D  (array i32))
  (type $D' (array i32))
  (type $B  (array (mut i32)))
  (type $B' (array (mut i32)))
  (type $E  (array i64))
  (type $E' (array i64))
  (type $F  (array anyref))
  (type $F' (array anyref))
  (type $G  (array (ref any)))
  (type $G' (array (ref any)))
  (type $H  (array (ref null $A)))
  (type $H' (array (ref null $A)))
  (type $I  (array (ref $A)))
  (type $I' (array (ref $A)))
  (type $J  (func (param i32 i32) (result i32 i32 i32)))
  (type $J' (func (param i32 i32) (result i32 i32 i32)))
  (type $K  (func (param i32 i32 i32) (result i32 i32)))
  (type $K' (func (param i32 i32 i32) (result i32 i32)))
  (type $L  (struct i32))
  (type $L' (struct i32))
  (type $M  (struct i32 i32))
  (type $M' (struct i32 i32))

  ;; CHECK:       (type $13 (func))

  ;; CHECK:      (func $foo (type $13)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $a' (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $B))
  ;; CHECK-NEXT:  (local $b' (ref null $B))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (local $c' (ref null $C))
  ;; CHECK-NEXT:  (local $d (ref null $D))
  ;; CHECK-NEXT:  (local $d' (ref null $D))
  ;; CHECK-NEXT:  (local $e (ref null $E))
  ;; CHECK-NEXT:  (local $e' (ref null $E))
  ;; CHECK-NEXT:  (local $f (ref null $F))
  ;; CHECK-NEXT:  (local $f' (ref null $F))
  ;; CHECK-NEXT:  (local $g (ref null $G))
  ;; CHECK-NEXT:  (local $g' (ref null $G))
  ;; CHECK-NEXT:  (local $h (ref null $H))
  ;; CHECK-NEXT:  (local $h' (ref null $H))
  ;; CHECK-NEXT:  (local $i (ref null $I))
  ;; CHECK-NEXT:  (local $i' (ref null $I))
  ;; CHECK-NEXT:  (local $j (ref null $J))
  ;; CHECK-NEXT:  (local $j' (ref null $J))
  ;; CHECK-NEXT:  (local $k (ref null $K))
  ;; CHECK-NEXT:  (local $k' (ref null $K))
  ;; CHECK-NEXT:  (local $l (ref null $L))
  ;; CHECK-NEXT:  (local $l' (ref null $L))
  ;; CHECK-NEXT:  (local $m (ref null $M))
  ;; CHECK-NEXT:  (local $m' (ref null $M))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    (local $a  (ref null $A))
    (local $a' (ref null $A'))
    (local $b  (ref null $B))
    (local $b' (ref null $B'))
    (local $c  (ref null $C))
    (local $c' (ref null $C'))
    (local $d  (ref null $D))
    (local $d' (ref null $D'))
    (local $e  (ref null $E))
    (local $e' (ref null $E'))
    (local $f  (ref null $F))
    (local $f' (ref null $F'))
    (local $g  (ref null $G))
    (local $g' (ref null $G'))
    (local $h  (ref null $H))
    (local $h' (ref null $H'))
    (local $i  (ref null $I))
    (local $i' (ref null $I'))
    (local $j  (ref null $J))
    (local $j' (ref null $J'))
    (local $k  (ref null $K))
    (local $k' (ref null $K'))
    (local $l  (ref null $L))
    (local $l' (ref null $L'))
    (local $m  (ref null $M))
    (local $m' (ref null $M'))
  )
)

(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $X (sub (struct (field anyref))))
  (type $X (sub (struct anyref)))
  ;; CHECK:       (type $Y (sub $X (struct (field eqref))))
  (type $Y (sub $X (struct eqref)))
  ;; CHECK:       (type $A (sub (struct (field (ref null $X)))))
  (type $A (sub (struct         (ref null $X))))
  ;; CHECK:       (type $B (sub $A (struct (field (ref null $Y)))))
  (type $B (sub $A (struct (ref null $Y))))
  (type $C (sub $A (struct (ref null $Y))))

  ;; CHECK:       (type $4 (func))

  ;; CHECK:      (func $foo (type $4)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $B))
  ;; CHECK-NEXT:  (local $c (ref null $B))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; B and C cannot be merged into A because they refine A's field, but B and
    ;; C can still be merged with each other.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $c (ref null $C))
  )
)

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct (field anyref))))
    (type $A (sub (struct    anyref)))
    (type $B (sub $A (struct eqref)))
    ;; CHECK:       (type $C (sub $A (struct (field eqref))))
    (type $C (sub $A (struct eqref)))
  )

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $C))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; This is the same as above, but now B and C refine A such that they have a
    ;; different top-level structure. They can still be merged.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $c (ref null $C))
  )
)

(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $A (sub (struct (field anyref))))
    (type $A (sub (struct    anyref)))
    (type $B (sub $A (struct anyref)))
    (type $C (sub $A (struct anyref)))
    (type $D (sub $B (struct eqref)))
    ;; CHECK:       (type $E (sub $A (struct (field eqref))))
    (type $E (sub $C (struct eqref)))
  )

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $A))
  ;; CHECK-NEXT:  (local $c (ref null $A))
  ;; CHECK-NEXT:  (local $d (ref null $E))
  ;; CHECK-NEXT:  (local $e (ref null $E))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; D and E should be mergeable because they have identical shapes and will
    ;; be siblings after B and C get merged.
    (local $a (ref null $A))
    (local $b (ref null $B))
    (local $c (ref null $C))
    (local $d (ref null $D))
    (local $e (ref null $E))
  )
)

;; Check that we fully optimize a type graph that requires multiple iterations
;; of supertype and sibling merging.
(module
  (rec
    ;; These will get merged in the initial supertype merging stage.
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $B' (sub (struct (field (ref $A)))))

    ;; CHECK:       (type $C (sub $B' (struct (field (ref $A)) (field i32))))

    ;; CHECK:       (type $D' (sub $C (struct (field (ref $A)) (field i32) (field i32))))

    ;; CHECK:       (type $A (sub (struct)))
    (type $A (sub (struct)))
    (type $A' (sub $A (struct)))

    ;; These siblings will be merged only after $a and $a' are merged.
    (type $B (sub (struct (ref $A))))
    (type $B' (sub (struct (ref $A'))))

    ;; These will get merged only after $b and $b' are merged.
    (type $C (sub $B (struct (ref $A) i32)))
    (type $C' (sub $B' (struct (ref $A') i32)))

    ;; These will get merged only after $c and $c' are merged.
    (type $D (sub $C (struct (ref $A) i32 i32)))
    (type $D' (sub $C' (struct (ref $A') i32 i32)))
  )

  ;; CHECK:       (type $4 (func))

  ;; CHECK:      (func $foo (type $4)
  ;; CHECK-NEXT:  (local $a (ref null $A))
  ;; CHECK-NEXT:  (local $a' (ref null $A))
  ;; CHECK-NEXT:  (local $b (ref null $B'))
  ;; CHECK-NEXT:  (local $b' (ref null $B'))
  ;; CHECK-NEXT:  (local $c (ref null $C))
  ;; CHECK-NEXT:  (local $c' (ref null $C))
  ;; CHECK-NEXT:  (local $d (ref null $D'))
  ;; CHECK-NEXT:  (local $d' (ref null $D'))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    (local $a (ref null $A))
    (local $a' (ref null $A'))
    (local $b (ref null $B))
    (local $b' (ref null $B'))
    (local $c (ref null $C))
    (local $c' (ref null $C'))
    (local $d (ref null $D))
    (local $d' (ref null $D'))
  )
)

;; Check that we refinalize properly.
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))
  (type $B (sub $A (struct)))

  ;; CHECK:       (type $1 (func (result (ref null $A))))

  ;; CHECK:      (func $returner (type $1) (result (ref null $A))
  ;; CHECK-NEXT:  (local $local (ref null $A))
  ;; CHECK-NEXT:  (local.get $local)
  ;; CHECK-NEXT: )
  (func $returner (result (ref null $B))
    (local $local (ref null $B))

    ;; After we change the local to use type $A, we need to update the local.get's
    ;; type as well, or we will error.
    (local.get $local)
  )
)

;; Test some real-world patterns, including fields to ignore, links between
;; merged types, etc.
;;
;; The result here is that we will merge $A$to-merge into $A, and $D$to-merge
;; into $D. While doing so we must update the fields and the expressions that
;; they appear in, and not error.
(module
  (rec
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $C (sub (struct (field (mut i32)))))

    ;; CHECK:       (type $D (sub $C (struct (field (mut i32)) (field (mut i32)))))

    ;; CHECK:       (type $H (sub $D (struct (field (mut i32)) (field (mut i32)) (field (mut (ref null $D))))))

    ;; CHECK:       (type $A (sub $H (struct (field (mut i32)) (field (mut i32)) (field (mut (ref null $D))) (field (mut i64)) (field (mut (ref null $I))))))

    ;; CHECK:       (type $I (array (mut (ref null $C))))
    (type $I (array (mut (ref null $C))))
    (type $C (sub (struct (field (mut i32)))))
    (type $D (sub $C (struct (field (mut i32)) (field (mut i32)))))
    (type $E (sub $D (struct (field (mut i32)) (field (mut i32)))))
    (type $F (sub $E (struct (field (mut i32)) (field (mut i32)))))
    (type $D$to-merge (sub $F (struct (field (mut i32)) (field (mut i32)))))
    ;; CHECK:       (type $G (func (param (ref $C)) (result (ref $D))))
    (type $G (func (param (ref $C)) (result (ref $D))))
    (type $H (sub $D (struct (field (mut i32)) (field (mut i32)) (field (mut (ref null $E))))))
    (type $A (sub $H (struct (field (mut i32)) (field (mut i32)) (field (mut (ref null $E))) (field (mut i64)) (field (mut (ref null $I))))))
    (type $A$to-merge (sub $A (struct (field (mut i32)) (field (mut i32)) (field (mut (ref null $E))) (field (mut i64)) (field (mut (ref null $I))))))
  )

  ;; CHECK:      (global $global$0 (ref $D) (struct.new $D
  ;; CHECK-NEXT:  (i32.const 1705)
  ;; CHECK-NEXT:  (i32.const 0)
  ;; CHECK-NEXT: ))
  (global $global$0 (ref $F) (struct.new $D$to-merge
    (i32.const 1705)
    (i32.const 0)
  ))
  ;; CHECK:      (func $0 (type $G) (param $0 (ref $C)) (result (ref $D))
  ;; CHECK-NEXT:  (struct.new $A
  ;; CHECK-NEXT:   (i32.const 1685)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:   (global.get $global$0)
  ;; CHECK-NEXT:   (i64.const 0)
  ;; CHECK-NEXT:   (array.new_fixed $I 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $0 (type $G) (param $0 (ref $C)) (result (ref $D))
    (struct.new $A$to-merge
      (i32.const 1685)
      (i32.const 0)
      (global.get $global$0)
      (i64.const 0)
      (array.new_fixed $I 0)
    )
  )
)

;; Arrays
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $refarray (sub (array anyref)))

  ;; CHECK:       (type $sub-refarray-nn (sub $refarray (array (ref any))))

  ;; CHECK:       (type $intarray (sub (array (mut i32))))
  (type $intarray (sub (array (mut i32))))
  (type $sub-intarray (sub $intarray (array (mut i32))))

  (type $refarray (sub (array (ref null any))))
  (type $sub-refarray    (sub $refarray (array (ref null any))))
  (type $sub-refarray-nn (sub $refarray (array (ref      any))))

  ;; CHECK:       (type $3 (func))

  ;; CHECK:      (func $foo (type $3)
  ;; CHECK-NEXT:  (local $a (ref null $intarray))
  ;; CHECK-NEXT:  (local $b (ref null $intarray))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; $A will remain the same.
    (local $a (ref null $intarray))
    ;; $B can be merged into $A.
    (local $b (ref null $sub-intarray))
  )

  ;; CHECK:      (func $bar (type $3)
  ;; CHECK-NEXT:  (local $a (ref null $refarray))
  ;; CHECK-NEXT:  (local $b (ref null $refarray))
  ;; CHECK-NEXT:  (local $c (ref null $sub-refarray-nn))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $bar
    (local $a (ref null $refarray))
    ;; $B can be merged into $A.
    (local $b (ref null $sub-refarray))
    ;; $C cannot be merged as the element type is more refined.
    (local $c (ref null $sub-refarray-nn))
  )
)

;; Function types
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $func (sub (func (param eqref))))
  (type $func (sub (func (param eqref))))
  (type $sub-func (sub $func (func (param eqref))))
  ;; CHECK:       (type $sub-func-refined (sub $func (func (param anyref))))
  (type $sub-func-refined (sub $func (func (param anyref))))

  ;; CHECK:       (type $2 (func))

  ;; CHECK:      (func $foo (type $2)
  ;; CHECK-NEXT:  (local $a (ref null $func))
  ;; CHECK-NEXT:  (local $b (ref null $func))
  ;; CHECK-NEXT:  (local $c (ref null $sub-func-refined))
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $foo
    ;; $func will remain the same.
    (local $a (ref null $func))
    ;; $sub-func will be merged into $func.
    (local $b (ref null $sub-func))
    ;; $sub-func-refined will not be merged into $func because it refines a result.
    (local $c (ref null $sub-func-refined))
  )
)

;; Check that public types are not merged.
(module
  ;; CHECK:      (type $A (sub (func)))
  (type $A (sub (func)))      ;; public
  ;; CHECK:      (type $B (sub $A (func)))
  (type $B (sub $A (func))) ;; public
  (type $C (sub $B (func))) ;; private

  ;; CHECK:      (type $2 (func (param (ref $A) (ref $B) (ref $B))))

  ;; CHECK:      (export "foo" (func $foo))
  (export "foo" (func $foo))
  ;; CHECK:      (export "bar" (func $bar))
  (export "bar" (func $bar))

  ;; A stays the same.
  ;; CHECK:      (func $foo (type $A)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $foo (type $A)
    (unreachable)
  )

  ;; B is not merged because it is public.
  ;; CHECK:      (func $bar (type $B)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $bar (type $B)
    (unreachable)
  )

  ;; C can be merged into B because it is private.
  ;; CHECK:      (func $baz (type $B)
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $baz (type $C)
    (unreachable)
  )

  ;; CHECK:      (func $quux (type $2) (param $0 (ref $A)) (param $1 (ref $B)) (param $2 (ref $B))
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $quux (param (ref $A) (ref $B) (ref $C))
    (unreachable)
  )
)

;; Regression test for a bug in which we tried to merge A into B instead of the
;; other way around, causing an assertion failure in type-updating.cpp.
(module
  (rec
    ;; CHECK:      (type $A (sub (func (param (ref null $A)) (result (ref null $A)))))
    (type $A (sub (func (param (ref null $B)) (result (ref null $A)))))
    (type $B (sub $A (func (param (ref null $A)) (result (ref null $B)))))
  )

  ;; CHECK:      (func $0 (type $A) (param $0 (ref null $A)) (result (ref null $A))
  ;; CHECK-NEXT:  (unreachable)
  ;; CHECK-NEXT: )
  (func $0 (type $B) (param $0 (ref null $A)) (result (ref null $B))
    (unreachable)
  )
)

;; Regresssion test for a bug in which we merged A into A', but
;; type-updating.cpp ordered B before A', so the supertype ordering was
;; incorrect.
(module
 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))
  (type $B (sub $A (struct)))
  ;; CHECK:       (type $X (struct (field (ref $A))))
  (type $X (struct (ref $B)))
  ;; CHECK:       (type $A' (struct))
  (type $A' (struct))
 )
 ;; CHECK:       (type $3 (func))

 ;; CHECK:      (func $foo (type $3)
 ;; CHECK-NEXT:  (local $b (ref null $A'))
 ;; CHECK-NEXT:  (local $x (ref null $X))
 ;; CHECK-NEXT:  (nop)
 ;; CHECK-NEXT: )
 (func $foo
   (local $b (ref null $A'))
   (local $x (ref null $X))
 )
)

;; Regression test for bug where we unsoundly merged supertypes and siblings in
;; a single step.
(module
  (rec
    ;; $x and $y are structurally identical, but won't be merged because there is
    ;; a cast to $y.
    ;; CHECK:      (rec
    ;; CHECK-NEXT:  (type $b (sub (struct (field (ref null $x)))))

    ;; CHECK:       (type $b1 (sub $b (struct (field (ref null $y)))))

    ;; CHECK:       (type $x (sub (struct (field anyref))))
    (type $x (sub (struct anyref)))
    ;; CHECK:       (type $y (sub $x (struct (field anyref))))
    (type $y (sub $x (struct anyref)))

    ;; If we did vertical and horizontal merges at the same time, these three
    ;; types would be put into the same initial partition and $b1 would be merged
    ;; into $a. This would be incorrect because then $b1 would no longer be a
    ;; subtype of $b.
    ;; CHECK:       (type $a (struct (field (ref null $y))))
    (type $a (struct (ref null $y)))
    (type $b (sub (struct (ref null $x))))
    (type $b1 (sub $b (struct (ref null $y))))
  )

  ;; CHECK:       (type $5 (func (result (ref $b))))

  ;; CHECK:      (func $test (type $5) (result (ref $b))
  ;; CHECK-NEXT:  (local $0 (ref null $a))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (ref.test (ref $y)
  ;; CHECK-NEXT:    (struct.new_default $x)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (struct.new_default $b1)
  ;; CHECK-NEXT: )
  (func $test (result (ref $b))
    ;; Use $a to prevent it from being dropped completely.
    (local (ref null $a))

    ;; Cast to prevent $x and $y from being merged.
    (drop
      (ref.test (ref $y)
        (struct.new_default $x)
      )
    )

    ;; If $b1 were merged with $a, this would cause a validation failure.
    (struct.new_default $b1)
  )
)

;; Regression test for a bug where we updated module types before building the
;; new types, causing the set of private types to change unexpectedly and
;; leading to a failure to build new types.
;; TODO: Store a heap type on control flow structures to avoid creating
;; standalone function types for them.
;; TODO: Investigate why the rec group contains two of the same type below.
(module
 (rec
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $B (sub (func)))

  ;; CHECK:       (type $A (sub (func (result (ref any) (ref $B)))))
  (type $A (sub (func (result (ref any) (ref $C)))))
  (type $B (sub (func)))
  (type $C (sub $B (func)))
  ;; CHECK:       (type $D (sub final $A (func (result (ref any) (ref $B)))))
  (type $D (sub final $A (func (result (ref any) (ref $C)))))
 )

 ;; CHECK:      (func $test (type $D) (result (ref any) (ref $B))
 ;; CHECK-NEXT:  (block $l (type $A) (result (ref any) (ref $B))
 ;; CHECK-NEXT:   (unreachable)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $test (type $D) (result (ref any) (ref $C))
  (block $l (result (ref any) (ref $C))
   (unreachable)
  )
 )
)

;; Regression test for a bug where we were over-aggressive in merging during the
;; supertype phase. Types A, B, C, D1, and D2 will all start out in the same
;; supertype merging partition, but partition refinement will show that A, B,
;; and C are distinct. We previously continued to merge D1 and D2, but that is
;; incorrect, as such a merge will either make g1 or g2 invalid below. The fix
;; was to manually split partitions that end up containing separate type trees.
(module
 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $I (sub (struct (field anyref))))
 (type $I (sub    (struct (field anyref))))
 ;; CHECK:       (type $J (sub $I (struct (field eqref))))
 (type $J (sub $I (struct (field eqref))))
 ;; CHECK:       (type $K (sub $J (struct (field i31ref))))
 (type $K (sub $J (struct (field i31ref))))
 (rec
  ;; CHECK:       (type $A (sub (struct (field (ref null $A)) (field (ref null $I)))))
  (type $A  (sub    (struct (ref null $A) (ref null $I))))
  ;; CHECK:       (type $C (sub $A (struct (field (ref null $A)) (field (ref null $K)))))

  ;; CHECK:       (type $D2 (sub $C (struct (field (ref null $B)) (field (ref null $K)))))

  ;; CHECK:       (type $B (sub $A (struct (field (ref null $B)) (field (ref null $J)))))
  (type $B  (sub $A (struct (ref null $B) (ref null $J))))
  (type $C  (sub $A (struct (ref null $A) (ref null $K))))
  ;; CHECK:       (type $D1 (sub $B (struct (field (ref null $B)) (field (ref null $K)))))
  (type $D1 (sub $B (struct (ref null $B) (ref null $K))))
  (type $D2 (sub $C (struct (ref null $B) (ref null $K))))
 )

 ;; CHECK:      (global $g1 (ref $B) (struct.new_default $D1))
 (global $g1 (ref $B) (struct.new_default $D1))
 ;; CHECK:      (global $g2 (ref $C) (struct.new_default $D2))
 (global $g2 (ref $C) (struct.new_default $D2))
)

;; Same as above, but with some additional types that can be merged.
(module
 ;; CHECK:      (rec
 ;; CHECK-NEXT:  (type $I (sub (struct (field anyref))))
 (type $I (sub    (struct (field anyref))))
 ;; CHECK:       (type $J (sub $I (struct (field eqref))))
 (type $J (sub $I (struct (field eqref))))
 ;; CHECK:       (type $K (sub $J (struct (field i31ref))))
 (type $K (sub $J (struct (field i31ref))))
 (rec
  ;; CHECK:       (type $A (sub (struct (field (ref null $A)) (field (ref null $I)))))
  (type $A  (sub     (struct (ref null $A) (ref null $I))))
  (type $A' (sub $A  (struct (ref null $A) (ref null $I))))
  ;; CHECK:       (type $C (sub $A (struct (field (ref null $A)) (field (ref null $K)))))

  ;; CHECK:       (type $D2 (sub $C (struct (field (ref null $B)) (field (ref null $K)))))

  ;; CHECK:       (type $B (sub $A (struct (field (ref null $B)) (field (ref null $J)))))
  (type $B  (sub $A' (struct (ref null $B) (ref null $J))))
  (type $B' (sub $B  (struct (ref null $B) (ref null $J))))
  (type $C  (sub $A' (struct (ref null $A) (ref null $K))))
  (type $C' (sub $C  (struct (ref null $A) (ref null $K))))
  ;; CHECK:       (type $D1 (sub $B (struct (field (ref null $B)) (field (ref null $K)))))
  (type $D1 (sub $B' (struct (ref null $B) (ref null $K))))
  (type $D1' (sub $D1 (struct (ref null $B) (ref null $K))))
  (type $D2 (sub $C' (struct (ref null $B) (ref null $K))))
  (type $D2' (sub $D2 (struct (ref null $B) (ref null $K))))
 )

 ;; CHECK:      (global $g1 (ref $B) (struct.new_default $D1))
 (global $g1 (ref $B') (struct.new_default $D1'))
 ;; CHECK:      (global $g2 (ref $C) (struct.new_default $D2))
 (global $g2 (ref $C') (struct.new_default $D2'))
)

;; Check that a ref.test inhibits merging (ref.cast is already checked above).
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))
  ;; CHECK:       (type $B (sub $A (struct)))
  (type $B (sub $A (struct)))

  ;; CHECK:       (type $2 (func (param (ref $A)) (result i32)))

  ;; CHECK:      (func $test (type $2) (param $a (ref $A)) (result i32)
  ;; CHECK-NEXT:  (ref.test (ref $B)
  ;; CHECK-NEXT:   (local.get $a)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $a (ref $A)) (result i32)
    (ref.test (ref $B)
      (local.get $a)
    )
  )
)

;; Check that a br_on_cast inhibits merging.
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (struct)))
  (type $A (sub (struct)))
  ;; CHECK:       (type $B (sub $A (struct)))
  (type $B (sub $A (struct)))

  ;; CHECK:       (type $2 (func (param (ref $A)) (result (ref $B))))

  ;; CHECK:      (func $test (type $2) (param $a (ref $A)) (result (ref $B))
  ;; CHECK-NEXT:  (block $label (result (ref $B))
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (br_on_cast $label (ref $A) (ref $B)
  ;; CHECK-NEXT:     (local.get $a)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (drop
  ;; CHECK-NEXT:    (block $l (result (ref $A))
  ;; CHECK-NEXT:     (br_on_non_null $l
  ;; CHECK-NEXT:      (local.get $a)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:     (unreachable)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (param $a (ref $A)) (result (ref $B))
    (drop
      (br_on_cast 0 anyref (ref $B)
        (local.get $a)
      )
    )
    ;; Also check that a different br_on* doesn't confuse us.
    (drop
      (block $l (result (ref $A))
        (br_on_non_null $l
          (local.get $a)
        )
        (unreachable)
      )
    )
    (unreachable)
  )
)

;; Check that a call_indirect inhibits merging.
(module
  ;; CHECK:      (rec
  ;; CHECK-NEXT:  (type $A (sub (func)))
  (type $A (sub (func)))
  ;; CHECK:       (type $B (sub $A (func)))
  (type $B (sub $A (func)))

  (table 1 1 (ref null $A))

  ;; CHECK:      (table $0 1 1 (ref null $A))

  ;; CHECK:      (func $test (type $A)
  ;; CHECK-NEXT:  (call_indirect $0 (type $B)
  ;; CHECK-NEXT:   (i32.const 0)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test (type $A)
    (call_indirect (type $B)
      (i32.const 0)
    )
  )
)
