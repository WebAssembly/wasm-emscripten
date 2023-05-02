;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-merge %s first %s.second second -all -S -o - | filecheck %s

;; Test we rename memories and data segments properly at the module scope.
;; Memory $bar has a name collision, and both of the element segments' names.

(module
  ;; CHECK:      (memory $foo 1)
  (memory $foo 1)

  ;; CHECK:      (memory $bar 10)
  (memory $bar 10)

  ;; CHECK:      (memory $other 100)

  ;; CHECK:      (memory $bar_2 1000)

  ;; CHECK:      (data $a (i32.const 0) "a")
  (data $a (memory $foo) (i32.const 0) "a")

  ;; CHECK:      (data $b (memory $bar) (i32.const 0) "b")
  (data $b (memory $bar) (i32.const 0) "b")
)
;; CHECK:      (data $a_2 (memory $other) (i32.const 0) "a2")

;; CHECK:      (data $b_2 (memory $bar_2) (i32.const 0) "b2")
