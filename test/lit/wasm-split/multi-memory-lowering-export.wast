;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s --multi-memory-lowering -all -S -o - | filecheck %s

(module
  (memory $memory1 1)
  (memory $memory2 1 1)
  (export "mem" (memory $memory1))
)

;; CHECK: (export "mem" (memory $combined_memory))
