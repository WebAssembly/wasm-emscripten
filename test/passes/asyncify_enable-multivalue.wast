;; Pre-existing imports that the pass turns into the implementations.
(module
  (memory 1 2)
  (import "asyncify" "start_unwind" (func $asyncify_start_unwind (param i32)))
  (import "asyncify" "stop_unwind" (func $asyncify_stop_unwind))
  (import "asyncify" "start_rewind" (func $asyncify_start_rewind (param i32)))
  (import "asyncify" "stop_rewind" (func $asyncify_stop_rewind))
  (global $sleeping (mut i32) (i32.const 0))
  ;; do a sleep operation: start a sleep if running, or resume after a sleep
  ;; if we just rewound.
  (func $do_sleep
    (if
      (i32.eqz (global.get $sleeping))
      (block
        (global.set $sleeping (i32.const 1))
        ;; we should set up the data at address 4 around here
        (call $asyncify_start_unwind (i32.const 4))
      )
      (block
        (global.set $sleeping (i32.const 0))
        (call $asyncify_stop_rewind)
      )
    )
  )
  ;; a function that does some work and has a sleep (async pause/resume) in the middle
  (func $work
    (call $stuff) ;; do some work
    (call $do_sleep) ;; take a break
    (call $stuff) ;; do some more work
  )
  (func  $stuff)
  ;; the first event called from the main event loop: just call into $work
  (func $first_event
    (call $work)
    ;; work will sleep, so we exit through here while it is paused
  )
  ;; the second event called from the main event loop: to resume $work,
  ;; stop the unwind, then prepare a rewind, and initiate it by doing
  ;; the call to rewind the call stack back up to where it was
  (func $second_event
    (call $asyncify_stop_unwind)
    (call $asyncify_start_rewind (i32.const 4))
    (call $work)
  )
  ;; a function that can't do a sleep
  (func $never_sleep
    (call $stuff)
    (call $stuff)
    (call $stuff)
  )
)
;; Calls to imports that will call into asyncify themselves.
(module
  (memory 1 2)
  (import "env" "import" (func $import))
  (import "env" "import2" (func $import2 (result i32)))
  (import "env" "import3" (func $import3 (param i32)))
  (import "env" "import-mv" (func $import-mv (result i32 i64)))
  (func $calls-import
    (call $import)
  )
  (func $calls-import2 (result i32)
    (local $temp i32)
    (local.set $temp (call $import2))
    (return (local.get $temp))
  )
  (func $calls-import2-drop
    (drop (call $import2))
  )
  (func $calls-nothing
    (drop (i32.eqz (i32.const 17)))
  )
  (func $many-locals (param $x i32) (result i32)
    (local $y i32)
    (local $z (f32 i64))
    (loop $l
      (local.set $x
        (i32.add (local.get $y) (i32.const 1))
      )
      (local.set $y
        (i32.div_s (local.get $x) (i32.const 3))
      )
      (br_if $l (local.get $y))
    )
    (call $import)
    (return (local.get $y))
  )
  (func $calls-import2-if (param $x i32)
    (if (local.get $x)
      (call $import)
    )
  )
  (func $calls-import2-if-else (param $x i32)
    (if (local.get $x)
      (call $import3 (i32.const 1))
      (call $import3 (i32.const 2))
    )
  )
  (func $calls-import2-if-else-oneside (param $x i32) (result i32)
    (if (local.get $x)
      (return (i32.const 1))
      (call $import3 (i32.const 2))
    )
    (return (i32.const 3))
  )
  (func $calls-import2-if-else-oneside2 (param $x i32) (result i32)
    (if (local.get $x)
      (call $import3 (i32.const 1))
      (return (i32.const 2))
    )
    (return (i32.const 3))
  )
  (func $calls-mv
    (local $x (i32 i64))
    (local.set $x (call $import-mv))
  )
  (func $calls-loop (param $x i32)
    (loop $l
      (call $import3 (i32.const 1))
      (local.set $x
        (i32.add (local.get $x) (i32.const 1))
      )
      (br_if $l
        (local.get $x)
      )
    )
  )
  (func $calls-loop2
    (loop $l
      (br_if $l
        (call $import2)
      )
    )
  )
  (func $calls-mix
    (call $boring)
    (call $import)
    (call $boring)
    (call $import)
  )
  (func $boring)
  (func $calls-mix-deep
    (call $boring-deep)
    (call $import-deep)
    (call $boring)
    (call $import)
  )
  (func $boring-deep
    (call $boring)
  )
  (func $import-deep
    (call $import)
  )
)
;; empty module, in particular with no memory
(module
)
;; module with a call result type that does not exist as a local
(module
 (import "fuzzing-support" "log-i32" (func $fimport$0 (param i32)))
 (func $0 (; 1 ;) (param $0 i32) (param $1 f64) (param $2 i32) (param $3 i64) (result f32)
  (call $fimport$0
   (i32.const 0)
  )
  (f32.const 1462441600)
 )
 (func $1 (; 2 ;) (param $0 i32) (param $1 f64) (param $2 i32) (param $3 i32) (param $4 i32) (result f32)
  (call $0
   (i32.const 0)
   (f64.const 1)
   (i32.const 1)
   (i64.const 1)
  )
 )
)
