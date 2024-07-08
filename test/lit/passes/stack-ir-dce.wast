;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt %s --generate-stack-ir --optimize-stack-ir -all --print-stack-ir | filecheck %s
;; Also verify we roundtrip the output here properly.
;; RUN: wasm-opt %s --generate-stack-ir --optimize-stack-ir -all --roundtrip --print | filecheck %s --check-prefix=ROUNDTRIP

(module
  ;; CHECK:      (func $drop-unreachable (type $0) (result i32)
  ;; CHECK-NEXT:  call $drop-unreachable
  ;; CHECK-NEXT:  unreachable
  ;; CHECK-NEXT: )
  ;; ROUNDTRIP:      (func $drop-unreachable (type $0) (result i32)
  ;; ROUNDTRIP-NEXT:  (drop
  ;; ROUNDTRIP-NEXT:   (call $drop-unreachable)
  ;; ROUNDTRIP-NEXT:  )
  ;; ROUNDTRIP-NEXT:  (unreachable)
  ;; ROUNDTRIP-NEXT: )
  (func $drop-unreachable (result i32)
    ;; This drop can be removed.
    (drop
      (call $drop-unreachable)
    )
    (unreachable)
  )
)
