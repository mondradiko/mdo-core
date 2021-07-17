(module
  (import "" "hello" (func $1 (param i32 i32) (result i32)))
  (memory (export "memory") 1)
  (global $nextFreeMemoryAddress (mut i32) (i32.const 0))
  (global $timeElapsed (mut f32) (f32.const 0))
  (func (export "malloc") (param $numBytes i32) (result i32)
  (local $address i32)
  (local.set $address (global.get $nextFreeMemoryAddress))
  (global.set $nextFreeMemoryAddress
    (i32.add (local.get $address) (local.get $numBytes)))
  (local.get $address)
  )
  (func (export "run") (param $address i32) (param $num_chars i32) (result i32)
  (call $1 (local.get $address) (local.get $num_chars))
  )
  (func (export "update") (param $dt f32) (result f32)
  (global.set $timeElapsed (f32.add (global.get $timeElapsed) (local.get $dt)))
  (global.get $timeElapsed)
  )
)
