;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all -o %t.text.wast -g -S
;; RUN: wasm-as %s -all -g -o %t.wasm
;; RUN: wasm-dis %t.wasm -all -o %t.bin.wast
;; RUN: wasm-as %s -all -o %t.nodebug.wasm
;; RUN: wasm-dis %t.nodebug.wasm -all -o %t.bin.nodebug.wast
;; RUN: cat %t.text.wast | filecheck %s --check-prefix=CHECK-TEXT
;; RUN: cat %t.bin.wast | filecheck %s --check-prefix=CHECK-BIN
;; RUN: cat %t.bin.nodebug.wast | filecheck %s --check-prefix=CHECK-BIN-NODEBUG

(module
  ;;@ src.cpp:1:1
  (func
    (nop)
    ;;@ src.cpp:2:1
    (block $l0
      ;;@ src.cpp:2:2
      (block $l1
        (br $l1)
      )
    )
    ;;@ src.cpp:3:1
    (return)
    ;;@ src.cpp:3:2
  )
)
;; CHECK-TEXT:      (type $0 (func))

;; CHECK-TEXT:      (func $0 (type $0)
;; CHECK-TEXT-NEXT:  (nop)
;; CHECK-TEXT-NEXT:  ;;@ src.cpp:2:1
;; CHECK-TEXT-NEXT:  (block $l0
;; CHECK-TEXT-NEXT:   ;;@ src.cpp:2:2
;; CHECK-TEXT-NEXT:   (block $l1
;; CHECK-TEXT-NEXT:    (br $l1)
;; CHECK-TEXT-NEXT:   )
;; CHECK-TEXT-NEXT:  )
;; CHECK-TEXT-NEXT:  ;;@ src.cpp:3:1
;; CHECK-TEXT-NEXT:  (return)
;; CHECK-TEXT-NEXT:  ;;@ src.cpp:3:2
;; CHECK-TEXT-NEXT: )

;; CHECK-BIN:      (type $0 (func))

;; CHECK-BIN:      (func $f0 (type $0)
;; CHECK-BIN-NEXT:  (nop)
;; CHECK-BIN-NEXT:  (block $label$1
;; CHECK-BIN-NEXT:   (block $label$2
;; CHECK-BIN-NEXT:    (br $label$2)
;; CHECK-BIN-NEXT:   )
;; CHECK-BIN-NEXT:  )
;; CHECK-BIN-NEXT:  (return)
;; CHECK-BIN-NEXT: )

;; CHECK-BIN-NODEBUG:      (type $0 (func))

;; CHECK-BIN-NODEBUG:      (func $f0 (type $0)
;; CHECK-BIN-NODEBUG-NEXT:  (nop)
;; CHECK-BIN-NODEBUG-NEXT:  (block $label$1
;; CHECK-BIN-NODEBUG-NEXT:   (block $label$2
;; CHECK-BIN-NODEBUG-NEXT:    (br $label$2)
;; CHECK-BIN-NODEBUG-NEXT:   )
;; CHECK-BIN-NODEBUG-NEXT:  )
;; CHECK-BIN-NODEBUG-NEXT:  (return)
;; CHECK-BIN-NODEBUG-NEXT: )
