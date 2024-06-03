;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt %s -all --string-lowering-magic-imports --remove-unused-module-elements -S -o - | filecheck %s
;; RUN: wasm-opt %s -all --string-lowering-magic-imports --remove-unused-module-elements --roundtrip -S -o - | filecheck %s --check-prefix=RTRIP

(module
  ;; CHECK:      (type $0 (func))

  ;; CHECK:      (import "\'" "bar" (global $string.const_bar (ref extern)))

  ;; CHECK:      (import "\'" "foo" (global $string.const_foo (ref extern)))

  ;; CHECK:      (import "\'" "needs\tescaping\00.\'#%- .\r\n\\08\0c\n\r\t.\ea\99\ae" (global $"string.const_needs\tescaping\00.\'#%- .\r\n\\08\0c\n\r\t.\ea\99\ae" (ref extern)))

  ;; CHECK:      (import "string.const" "0" (global $"string.const_unpaired high surrogate \ed\a0\80 " (ref extern)))

  ;; CHECK:      (import "string.const" "1" (global $"string.const_unpaired low surrogate \ed\bd\88 " (ref extern)))

  ;; CHECK:      (export "consts" (func $consts))

  ;; CHECK:      (func $consts (type $0)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $string.const_foo)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $string.const_bar)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $"string.const_needs\tescaping\00.\'#%- .\r\n\\08\0c\n\r\t.\ea\99\ae")
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $"string.const_unpaired high surrogate \ed\a0\80 ")
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (global.get $"string.const_unpaired low surrogate \ed\bd\88 ")
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  ;; RTRIP:      (type $0 (func))

  ;; RTRIP:      (import "\'" "bar" (global $gimport$0 (ref extern)))

  ;; RTRIP:      (import "\'" "foo" (global $gimport$1 (ref extern)))

  ;; RTRIP:      (import "\'" "needs\tescaping\00.\'#%- .\r\n\\08\0c\n\r\t.\ea\99\ae" (global $gimport$2 (ref extern)))

  ;; RTRIP:      (import "string.const" "0" (global $gimport$3 (ref extern)))

  ;; RTRIP:      (import "string.const" "1" (global $gimport$4 (ref extern)))

  ;; RTRIP:      (export "consts" (func $consts))

  ;; RTRIP:      (func $consts (type $0)
  ;; RTRIP-NEXT:  (drop
  ;; RTRIP-NEXT:   (global.get $gimport$1)
  ;; RTRIP-NEXT:  )
  ;; RTRIP-NEXT:  (drop
  ;; RTRIP-NEXT:   (global.get $gimport$0)
  ;; RTRIP-NEXT:  )
  ;; RTRIP-NEXT:  (drop
  ;; RTRIP-NEXT:   (global.get $gimport$2)
  ;; RTRIP-NEXT:  )
  ;; RTRIP-NEXT:  (drop
  ;; RTRIP-NEXT:   (global.get $gimport$3)
  ;; RTRIP-NEXT:  )
  ;; RTRIP-NEXT:  (drop
  ;; RTRIP-NEXT:   (global.get $gimport$4)
  ;; RTRIP-NEXT:  )
  ;; RTRIP-NEXT: )
  (func $consts (export "consts")
    (drop
      (string.const "foo")
    )
    (drop
      (string.const "bar")
    )
    (drop
      (string.const "needs\tescaping\00.'#%- .\r\n\\08\0C\0A\0D\09.ꙮ")
    )
    (drop
      (string.const "unpaired high surrogate \ED\A0\80 ")
    )
    (drop
      (string.const "unpaired low surrogate \ED\BD\88 ")
    )
  )
)
