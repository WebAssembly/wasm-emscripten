;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.

;; RUN: wasm-opt %s --precompute -all -S -o - | filecheck %s

(module
 ;; CHECK:      (func $eq-no (type $0) (result i32)
 ;; CHECK-NEXT:  (i32.const 0)
 ;; CHECK-NEXT: )
 (func $eq-no (result i32)
  (string.eq
   (string.const "ab")
   (string.const "cdefg")
  )
 )

 ;; CHECK:      (func $eq-yes (type $0) (result i32)
 ;; CHECK-NEXT:  (i32.const 1)
 ;; CHECK-NEXT: )
 (func $eq-yes (result i32)
  (string.eq
   (string.const "ab")
   (string.const "ab")
  )
 )

 ;; CHECK:      (func $concat (type $0) (result i32)
 ;; CHECK-NEXT:  (i32.const 1)
 ;; CHECK-NEXT: )
 (func $concat (result i32)
  (string.eq
   (string.concat (string.const "a") (string.const "b"))
   (string.const "ab")
  )
 )

 ;; CHECK:      (func $length (type $0) (result i32)
 ;; CHECK-NEXT:  (i32.const 7)
 ;; CHECK-NEXT: )
 (func $length (result i32)
  (stringview_wtf16.length
   (string.as_wtf16
    (string.const "1234567")
   )
  )
 )

 ;; CHECK:      (func $length-bad (type $0) (result i32)
 ;; CHECK-NEXT:  (stringview_wtf16.length
 ;; CHECK-NEXT:   (string.as_wtf16
 ;; CHECK-NEXT:    (string.const "$_\c2\a3_\e2\82\ac_\f0\90\8d\88")
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $length-bad (result i32)
  ;; Not precomputable because we don't handle unicode yet.
  (stringview_wtf16.length
   (string.as_wtf16
    ;; $_£_€_𐍈
    (string.const "$_\C2\A3_\E2\82\AC_\F0\90\8D\88")
   )
  )
 )

 ;; CHECK:      (func $get_codepoint (type $0) (result i32)
 ;; CHECK-NEXT:  (i32.const 95)
 ;; CHECK-NEXT: )
 (func $get_codepoint (result i32)
  ;; This is computable because everything up to the requested index is ascii. Returns 95 ('_').
  (stringview_wtf16.get_codeunit
   (string.as_wtf16
    ;; $_£_€_𐍈
    (string.const "$_\C2\A3_\E2\82\AC_\F0\90\8D\88")
   )
   (i32.const 1)
  )
 )

 ;; CHECK:      (func $get_codepoint-bad (type $0) (result i32)
 ;; CHECK-NEXT:  (stringview_wtf16.get_codeunit
 ;; CHECK-NEXT:   (string.as_wtf16
 ;; CHECK-NEXT:    (string.const "$_\c2\a3_\e2\82\ac_\f0\90\8d\88")
 ;; CHECK-NEXT:   )
 ;; CHECK-NEXT:   (i32.const 2)
 ;; CHECK-NEXT:  )
 ;; CHECK-NEXT: )
 (func $get_codepoint-bad (export "get_codepoint-bad") (result i32)
  ;; This is not computable because the requested code unit is not ascii.
  (stringview_wtf16.get_codeunit
   (string.as_wtf16
    ;; $_£_€_𐍈
    (string.const "$_\C2\A3_\E2\82\AC_\F0\90\8D\88")
   )
   (i32.const 2)
  )
 )
)
