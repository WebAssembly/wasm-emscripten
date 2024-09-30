;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: foreach %s %t wasm-opt -all --remove-unused-brs -S -o - | filecheck %s

(module
  ;; CHECK:      (tag $e)
  (tag $e)
  ;; CHECK:      (tag $f)
  (tag $f)
  ;; CHECK:      (tag $g)
  (tag $g)

  ;; CHECK:      (func $throw-caught-all (type $0)
  ;; CHECK-NEXT:  (block $catch
  ;; CHECK-NEXT:   (try_table (catch_all $catch)
  ;; CHECK-NEXT:    (br $catch)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-all
    (block $catch
      (try_table (catch_all $catch)
        ;; This throw can be a br.
        (throw $e)
      )
    )
  )

  ;; CHECK:      (func $throw-caught-all-more (type $2) (param $x i32)
  ;; CHECK-NEXT:  (block $catch
  ;; CHECK-NEXT:   (try_table (catch_all $catch)
  ;; CHECK-NEXT:    (br_if $catch
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-all-more (param $x i32)
    (block $catch
      (try_table (catch_all $catch)
        ;; Look into nested children. After we turn the throw into a br, it also
        ;; fuses with the if into a br_if.
        (if
          (local.get $x)
          (then
            (throw $e)
          )
        )
      )
    )
  )

  ;; CHECK:      (func $throw-caught-precise (type $0)
  ;; CHECK-NEXT:  (block $catch
  ;; CHECK-NEXT:   (try_table (catch $e $catch)
  ;; CHECK-NEXT:    (br $catch)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-precise
    (block $catch
      ;; We can still optimize here, even though we replaced the catch_all with
      ;; a precise tag, because the tag matches.
      (try_table (catch $e $catch)
        (throw $e)
      )
    )
  )

  ;; CHECK:      (func $throw-caught-precise-later (type $0)
  ;; CHECK-NEXT:  (block $fail
  ;; CHECK-NEXT:   (block $catch
  ;; CHECK-NEXT:    (try_table (catch $f $fail) (catch $e $catch)
  ;; CHECK-NEXT:     (br $catch)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $throw-caught-precise-later)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-precise-later
    (block $fail
      (block $catch
        ;; We can still optimize here, by looking through the tags til we find
        ;; ours.
        (try_table (catch $f $fail) (catch $e $catch)
          (throw $e)
        )
      )
      ;; Add an effect here, so the two blocks are not mergeable.
      (call $throw-caught-precise-later)
    )
  )

  ;; CHECK:      (func $throw-caught-all-later (type $0)
  ;; CHECK-NEXT:  (block $fail
  ;; CHECK-NEXT:   (block $catch
  ;; CHECK-NEXT:    (try_table (catch $f $fail) (catch_all $catch)
  ;; CHECK-NEXT:     (br $catch)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $throw-caught-precise-later)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-all-later
    (block $fail
      (block $catch
        ;; We can still optimize here, by looking through the tags til we find
        ;; the catch_all
        (try_table (catch $f $fail) (catch_all $catch)
          (throw $e)
        )
      )
      ;; Add an effect here, so the two blocks are not mergeable.
      (call $throw-caught-precise-later)
    )
  )

  ;; CHECK:      (func $throw-caught-fail (type $0)
  ;; CHECK-NEXT:  (block $catch
  ;; CHECK-NEXT:   (try_table (catch $f $catch)
  ;; CHECK-NEXT:    (throw $e)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-fail
    (block $catch
      ;; The tag does *not* match.
      (try_table (catch $f $catch)
        (throw $e)
      )
    )
  )

  ;; CHECK:      (func $throw-caught-outer (type $0)
  ;; CHECK-NEXT:  (block $fail
  ;; CHECK-NEXT:   (block $catch
  ;; CHECK-NEXT:    (try_table (catch $e $catch)
  ;; CHECK-NEXT:     (try_table (catch $f $fail)
  ;; CHECK-NEXT:      (br $catch)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $throw-caught-precise-later)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-outer
    (block $fail
      (block $catch
        (try_table (catch $e $catch)
          (try_table (catch $f $fail)
            ;; This throw can be a br, thanks to the outer try.
            (throw $e)
          )
        )
      )
      ;; Add an effect here, so the two blocks are not mergeable.
      (call $throw-caught-precise-later)
    )
  )

  ;; CHECK:      (func $throw-catch-all-ref (type $1) (result exnref)
  ;; CHECK-NEXT:  (block $catch (result exnref)
  ;; CHECK-NEXT:   (try_table (catch_all_ref $catch)
  ;; CHECK-NEXT:    (throw $e)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-catch-all-ref (result exnref)
    (block $catch (result exnref)
      (try_table (catch_all_ref $catch)
        ;; This is caught with a ref, so we do not optimize.
        (throw $e)
      )
    )
  )

  ;; CHECK:      (func $throw-caught-ref (type $1) (result exnref)
  ;; CHECK-NEXT:  (block $catch (result exnref)
  ;; CHECK-NEXT:   (try_table (catch_ref $e $catch)
  ;; CHECK-NEXT:    (throw $e)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-ref (result exnref)
    (block $catch (result exnref)
      (try_table (catch_ref $e $catch)
        ;; As above, but with catch_all.
        (throw $e)
      )
    )
  )

  ;; CHECK:      (func $throw-caught-ref-later-all (type $1) (result exnref)
  ;; CHECK-NEXT:  (block $outer (result exnref)
  ;; CHECK-NEXT:   (block $catch
  ;; CHECK-NEXT:    (try_table (catch_ref $e $outer) (catch_all $catch)
  ;; CHECK-NEXT:     (throw $e)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (unreachable)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-ref-later-all (result exnref)
    (block $outer (result exnref)
      (block $catch
        (try_table (catch_ref $e $outer) (catch_all $catch)
          ;; This is caught with a ref, before we reach the catch all, so we do
          ;; not optimize.
          (throw $e)
        )
      )
      (unreachable)
    )
  )

  ;; CHECK:      (func $throw-multi (type $0)
  ;; CHECK-NEXT:  (block $outer
  ;; CHECK-NEXT:   (block $middle
  ;; CHECK-NEXT:    (block $inner
  ;; CHECK-NEXT:     (try_table (catch $e $outer) (catch $f $middle) (catch_all $inner)
  ;; CHECK-NEXT:      (br $outer)
  ;; CHECK-NEXT:      (br $middle)
  ;; CHECK-NEXT:      (br $inner)
  ;; CHECK-NEXT:     )
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (call $throw-caught-precise-later)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:   (call $throw-caught-precise-later)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-multi
    (block $outer
      (block $middle
        (block $inner
          (try_table (catch $e $outer) (catch $f $middle) (catch_all $inner)
            ;; Multiple throws, optimizable in different ways.
            (throw $e)
            (throw $f)
            (throw $g)
          )
        )
        ;; Add an effect here, so the two blocks are not mergeable.
        (call $throw-caught-precise-later)
      )
      (call $throw-caught-precise-later)
    )
  )
)

;; Test dropping of throw children.
(module
  ;; CHECK:      (import "a" "b" (func $effect (type $1) (result i32)))
  (import "a" "b" (func $effect (result i32)))

  ;; CHECK:      (tag $e (param i32))
  (tag $e (param i32))

  ;; CHECK:      (func $throw-caught-all (type $0) (param $x i32)
  ;; CHECK-NEXT:  (block $catch
  ;; CHECK-NEXT:   (try_table (catch_all $catch)
  ;; CHECK-NEXT:    (drop
  ;; CHECK-NEXT:     (call $effect)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:    (br $catch)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $throw-caught-all (param $x i32)
    (block $catch
      (try_table (catch_all $catch)
        ;; This throw can be a br. The call must be kept in a drop.
        (throw $e
          (call $effect)
        )
      )
    )
  )
)
