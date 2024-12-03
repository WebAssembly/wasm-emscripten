;; NOTE: Assertions have been generated by update_lit_checks.py --output=fuzz-exec and should not be edited.

;; RUN: wasm-opt %s -all --fuzz-exec -o /dev/null 2>&1 | filecheck %s

;; Test the fuzzing-support module imports.

(module
 (import "fuzzing-support" "log-i32" (func $log-i32 (param i32)))
 (import "fuzzing-support" "log-f64" (func $log-f64 (param f64)))

 (import "fuzzing-support" "throw" (func $throw))

 (import "fuzzing-support" "table-set" (func $table.set (param i32 funcref)))
 (import "fuzzing-support" "table-get" (func $table.get (param i32) (result funcref)))

 (import "fuzzing-support" "call-export" (func $call.export (param i32)))
 (import "fuzzing-support" "call-export-catch" (func $call.export.catch (param i32) (result i32)))

 (import "fuzzing-support" "call-ref" (func $call.ref (param funcref)))
 (import "fuzzing-support" "call-ref-catch" (func $call.ref.catch (param funcref) (result i32)))

 (table $table 10 20 funcref)

 ;; Note that the exported table appears first here, but in the binary and in
 ;; the IR it is actually last, as we always add function exports first.
 (export "table" (table $table))

 ;; CHECK:      [fuzz-exec] calling logging
 ;; CHECK-NEXT: [LoggingExternalInterface logging 42]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
 (func $logging (export "logging")
  (call $log-i32
   (i32.const 42)
  )
  (call $log-f64
   (f64.const 3.14159)
  )
 )

 ;; CHECK:      [fuzz-exec] calling throwing
 ;; CHECK-NEXT: [exception thrown: __private ()]
 (func $throwing (export "throwing")
  (call $throw)
 )

 ;; CHECK:      [fuzz-exec] calling table.setting
 ;; CHECK-NEXT: [exception thrown: __private ()]
 (func $table.setting (export "table.setting")
  (call $table.set
   (i32.const 5)
   (ref.func $table.setting)
  )
  ;; Out of bounds sets will throw.
  (call $table.set
   (i32.const 9999)
   (ref.func $table.setting)
  )
 )

 ;; CHECK:      [fuzz-exec] calling table.getting
 ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 ;; CHECK-NEXT: [exception thrown: __private ()]
 (func $table.getting (export "table.getting")
  ;; There is a non-null value at 5, and a null at 6.
  (call $log-i32
   (ref.is_null
    (call $table.get
     (i32.const 5)
    )
   )
  )
  (call $log-i32
   (ref.is_null
    (call $table.get
     (i32.const 6)
    )
   )
  )
  ;; Out of bounds gets will throw.
  (drop
   (call $table.get
    (i32.const 9999)
   )
  )
 )

 ;; CHECK:      [fuzz-exec] calling export.calling
 ;; CHECK-NEXT: [LoggingExternalInterface logging 42]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
 ;; CHECK-NEXT: [exception thrown: __private ()]
 (func $export.calling (export "export.calling")
  ;; At index 0 in the exports we have $logging, so we will do those loggings.
  (call $call.export
   (i32.const 0)
  )
  ;; At index 999 we have nothing, so we'll error.
  (call $call.export
   (i32.const 999)
  )
 )

 ;; CHECK:      [fuzz-exec] calling export.calling.catching
 ;; CHECK-NEXT: [LoggingExternalInterface logging 42]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 (func $export.calling.catching (export "export.calling.catching")
  ;; At index 0 in the exports we have $logging, so we will do those loggings,
  ;; then log a 0 as no exception happens.
  (call $log-i32
   (call $call.export.catch
    (i32.const 0)
   )
  )
  ;; At index 999 we have nothing, so we'll error, catch it, and log 1.
  (call $log-i32
   (call $call.export.catch
    (i32.const 999)
   )
  )
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling
 ;; CHECK-NEXT: [LoggingExternalInterface logging 42]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
 ;; CHECK-NEXT: [exception thrown: __private ()]
 (func $ref.calling (export "ref.calling")
  ;; This will emit some logging.
  (call $call.ref
   (ref.func $logging)
  )
  ;; This will trap.
  (call $call.ref
   (ref.null func)
  )
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.catching
 ;; CHECK-NEXT: [LoggingExternalInterface logging 42]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 (func $ref.calling.catching (export "ref.calling.catching")
  ;; This will emit some logging, then log 0 as we do not error.
  (call $log-i32
   (call $call.ref.catch
    (ref.func $logging)
   )
  )
  ;; The trap here is caught, and we'll log 1.
  (call $log-i32
   (call $call.ref.catch
    (ref.null func)
   )
  )
 )

 (func $legal (param $x i32) (result i32)
  ;; Helper for the function below. All types here are legal for JS.
  (call $log-i32
   (i32.const 12)
  )
  (i32.const 34)
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.legal
 ;; CHECK-NEXT: [LoggingExternalInterface logging 12]
 (func $ref.calling.legal (export "ref.calling.legal")
  ;; It is fine to call-ref a function with params and results. The params get
  ;; default values, and the results are ignored. All we will see here is the
  ;; logging from the function, "1234".
  (call $call.ref
   (ref.func $legal)
  )
 )

 (func $illegal (param $x i64)
  ;; Helper for the function below. The param, an i64, causes a problem: when we
  ;; call from JS we provide 0, but 0 traps when it tries to convert to BigInt.
  (call $log-i32
   (i32.const 56)
  )
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.illegal
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 (func $ref.calling.illegal (export "ref.calling.illegal")
  ;; The i64 param causes an error here, so we will only log 1 as a trap.
  (call $log-i32
   (call $call.ref.catch
    (ref.func $illegal)
   )
  )
 )

 (func $illegal-v128 (param $x v128)
  ;; Helper for the function below.
  (call $log-i32
   (i32.const 56)
  )
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.illegal-v128
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 (func $ref.calling.illegal-v128 (export "ref.calling.illegal-v128")
  ;; As above, we trap on the v128 param, and log 1.
  (call $log-i32
   (call $call.ref.catch
    (ref.func $illegal-v128)
   )
  )
 )

 (func $illegal-result (result v128)
  ;; Helper for the function below. The result is illegal for JS.
  (call $log-i32
   (i32.const 910)
  )
  (v128.const i32x4 1 2 3 4)
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.illegal-result
 ;; CHECK-NEXT: [LoggingExternalInterface logging 1]
 (func $ref.calling.illegal-result (export "ref.calling.illegal-result")
  ;; The v128 result causes an error here, so we will log 1 as a trap. The JS
  ;; semantics determine that we do that check *before* the call, so the logging
  ;; of 910 does not go through.
  (call $log-i32
   (call $call.ref.catch
    (ref.func $illegal-result)
   )
  )
 )

 (func $legal-result (result i64)
  ;; Helper for the function below.
  (call $log-i32
   (i32.const 910)
  )
  (i64.const 90)
 )

 ;; CHECK:      [fuzz-exec] calling ref.calling.legal-result
 ;; CHECK-NEXT: [LoggingExternalInterface logging 910]
 ;; CHECK-NEXT: [LoggingExternalInterface logging 0]
 ;; CHECK-NEXT: warning: no passes specified, not doing any work
 (func $ref.calling.legal-result (export "ref.calling.legal-result")
  ;; Unlike v128, i64 is legal in a result. The JS VM just returns a BigInt.
  (call $log-i32
   (call $call.ref.catch
    (ref.func $legal-result)
   )
  )
 )
)
;; CHECK:      [fuzz-exec] calling logging
;; CHECK-NEXT: [LoggingExternalInterface logging 42]
;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]

;; CHECK:      [fuzz-exec] calling throwing
;; CHECK-NEXT: [exception thrown: __private ()]

;; CHECK:      [fuzz-exec] calling table.setting
;; CHECK-NEXT: [exception thrown: __private ()]

;; CHECK:      [fuzz-exec] calling table.getting
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]
;; CHECK-NEXT: [exception thrown: __private ()]

;; CHECK:      [fuzz-exec] calling export.calling
;; CHECK-NEXT: [LoggingExternalInterface logging 42]
;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
;; CHECK-NEXT: [exception thrown: __private ()]

;; CHECK:      [fuzz-exec] calling export.calling.catching
;; CHECK-NEXT: [LoggingExternalInterface logging 42]
;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]

;; CHECK:      [fuzz-exec] calling ref.calling
;; CHECK-NEXT: [LoggingExternalInterface logging 42]
;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
;; CHECK-NEXT: [exception thrown: __private ()]

;; CHECK:      [fuzz-exec] calling ref.calling.catching
;; CHECK-NEXT: [LoggingExternalInterface logging 42]
;; CHECK-NEXT: [LoggingExternalInterface logging 3.14159]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [LoggingExternalInterface logging 1]

;; CHECK:      [fuzz-exec] calling ref.calling.legal
;; CHECK-NEXT: [LoggingExternalInterface logging 12]

;; CHECK:      [fuzz-exec] calling ref.calling.illegal
;; CHECK-NEXT: [LoggingExternalInterface logging 1]

;; CHECK:      [fuzz-exec] calling ref.calling.illegal-v128
;; CHECK-NEXT: [LoggingExternalInterface logging 1]

;; CHECK:      [fuzz-exec] calling ref.calling.illegal-result
;; CHECK-NEXT: [LoggingExternalInterface logging 1]

;; CHECK:      [fuzz-exec] calling ref.calling.legal-result
;; CHECK-NEXT: [LoggingExternalInterface logging 910]
;; CHECK-NEXT: [LoggingExternalInterface logging 0]
;; CHECK-NEXT: [fuzz-exec] comparing export.calling
;; CHECK-NEXT: [fuzz-exec] comparing export.calling.catching
;; CHECK-NEXT: [fuzz-exec] comparing logging
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.catching
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.illegal
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.illegal-result
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.illegal-v128
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.legal
;; CHECK-NEXT: [fuzz-exec] comparing ref.calling.legal-result
;; CHECK-NEXT: [fuzz-exec] comparing table.getting
;; CHECK-NEXT: [fuzz-exec] comparing table.setting
;; CHECK-NEXT: [fuzz-exec] comparing throwing
