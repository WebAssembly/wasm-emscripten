;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --generate-stack-ir --optimize-stack-ir --shrink-level=1 \
;; RUN:   -all --print-stack-ir | filecheck %s

;; Shrink level is set to 1 to enable local2stack in StackIR opts.

(module
 ;; CHECK:      (func $if (type $0) (param $param (ref eq)) (result (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT: )
 (func $if (param $param (ref eq)) (result (ref eq))
  (local $temp (ref eq))
  ;; Copy the param into $temp. $temp is then set in both arms of the if, so
  ;; it is set before the get at the end of the function, but we still need to
  ;; keep this set for validation purposes. Specifically, there is a set of
  ;; $temp followed by a get of it in the if condition, which local2stack could
  ;; remove in principle, if not for that final get at the function end.
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $if-no-last-get (type $0) (param $param (ref eq)) (result (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT: )
 (func $if-no-last-get (param $param (ref eq)) (result (ref eq))
  ;; As the original, but now there is no final get, so we can remove the set-
  ;; get pair of $temp before the if.
  (local $temp (ref eq))
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.get $param) ;; this changed from $temp to $param
 )

 ;; CHECK:      (func $if-extra-set (type $0) (param $param (ref eq)) (result (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT: )
 (func $if-extra-set (param $param (ref eq)) (result (ref eq))
  ;; As the original, but now there is an extra set before the final get, so
  ;; we can optimize - the extra set ensures validation.
  (local $temp (ref eq))
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.set $temp    ;; This set is new.
   (local.get $param)
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $if-wrong-extra-set (type $0) (param $param (ref eq)) (result (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $param
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT: )
 (func $if-wrong-extra-set (param $param (ref eq)) (result (ref eq))
  ;; As the last testcase, but the extra set's index is wrong, so we cannot
  ;; optimize.
  (local $temp (ref eq))
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.set $param    ;; This set now writes to $param.
   (local.get $param)
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $if-param (type $2) (param $param (ref eq)) (param $temp (ref eq)) (result (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT: )
 (func $if-param (param $param (ref eq)) (param $temp (ref eq)) (result (ref eq))
  ;; As the original testcase, but now $temp is a param. Validation is no
  ;; longer an issue, so we can optimize away the pair.
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $if-nullable (type $3) (param $param (ref eq)) (result eqref)
 ;; CHECK-NEXT:  (local $temp eqref)
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  i31.new
 ;; CHECK-NEXT:  ref.eq
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   i31.new
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT: )
 (func $if-nullable (param $param (ref eq)) (result (ref null eq))
  (local $temp (ref null eq)) ;; this changed
  ;; As the original testcase, but now $temp is a nullable. Validation is no
  ;; longer an issue, so we can optimize away the pair.
  (local.set $temp
   (local.get $param)
  )
  (if
   (ref.eq
    (local.get $temp)
    (i31.new
     (i32.const 0)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 1)
    )
   )
   (local.set $temp
    (i31.new
     (i32.const 2)
    )
   )
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $if-non-ref (type $4) (param $param i32) (result i32)
 ;; CHECK-NEXT:  (local $temp i32)
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  i32.eqz
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   i32.const 1
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   i32.const 2
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT: )
 (func $if-non-ref (param $param i32) (result i32)
  (local $temp i32)
  ;; As the original testcase, but now $temp is not a ref. Validation is no
  ;; longer an issue, so we can optimize away the pair.
  (local.set $temp
   (local.get $param)
  )
  (if
   (i32.eqz
    (local.get $temp)
    (i32.const 0)
   )
   (local.set $temp
    (i32.const 1)
   )
   (local.set $temp
    (i32.const 2)
   )
  )
  (local.get $temp)
 )

 ;; CHECK:      (func $nesting (type $1) (param $param (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT:  local.get $temp
 ;; CHECK-NEXT:  drop
 ;; CHECK-NEXT: )
 (func $nesting (param $param (ref eq))
  (local $temp (ref eq))
  ;; The if arms contain optimization opportunities, even though there are 2
  ;; gets in each one, because this top set helps them all validate. Atm we do
  ;; not look backwards, however, so we fail to optimize here.
  (local.set $temp
   (local.get $param)
  )
  (if
   (i32.const 0)
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    (drop
     (local.get $temp)
    )
   )
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    (drop
     (local.get $temp)
    )
   )
  )
  (drop
   (local.get $temp)
  )
 )

 ;; CHECK:      (func $nesting-left (type $1) (param $param (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT: )
 (func $nesting-left (param $param (ref eq))
  (local $temp (ref eq))
  ;; As $nesting, but now the left arm has one one get, and can be optimized.
  (local.set $temp
   (local.get $param)
  )
  (if
   (i32.const 0)
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    ;; A get was removed here.
   )
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    (drop
     (local.get $temp)
    )
   )
  )
 )

 ;; CHECK:      (func $nesting-right (type $1) (param $param (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   local.set $temp
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:   local.get $temp
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT: )
 (func $nesting-right (param $param (ref eq))
  (local $temp (ref eq))
  ;; As above, but now we can optimize the right arm.
  (local.set $temp
   (local.get $param)
  )
  (if
   (i32.const 0)
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    (drop
     (local.get $temp)
    )
   )
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    ;; A get was removed here.
   )
  )
 )

 ;; CHECK:      (func $nesting-both (type $1) (param $param (ref eq))
 ;; CHECK-NEXT:  (local $temp (ref eq))
 ;; CHECK-NEXT:  local.get $param
 ;; CHECK-NEXT:  local.set $temp
 ;; CHECK-NEXT:  i32.const 0
 ;; CHECK-NEXT:  if
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  else
 ;; CHECK-NEXT:   local.get $param
 ;; CHECK-NEXT:   drop
 ;; CHECK-NEXT:  end
 ;; CHECK-NEXT: )
 (func $nesting-both (param $param (ref eq))
  (local $temp (ref eq))
  ;; As above, but now we can optimize both arms.
  (local.set $temp
   (local.get $param)
  )
  (if
   (i32.const 0)
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    ;; A get was removed here.
   )
   (block
    (local.set $temp
     (local.get $param)
    )
    (drop
     (local.get $temp)
    )
    ;; A get was removed here.
   )
  )
 )
)

;; TODO: test nesting etc.
;; TODO: test param

;; TODO: test set before as a TODO
