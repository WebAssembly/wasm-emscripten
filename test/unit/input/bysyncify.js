
function assert(x, y) {
  if (!x) throw (y || 'assertion failed') + '\n' + new Error().stack;
}

var fs = require('fs');

// Get and compile the wasm.

var binary = fs.readFileSync('a.wasm');

var module = new WebAssembly.Module(binary);

var DATA_ADDR = 4;

var sleeps = 0;

var sleeping = false;

var instance = new WebAssembly.Instance(module, {
  env: {
    sleep: function() {
      logMemory();
assert(view[0] == 0);
      if (!sleeping) {
        // We are called in order to start a sleep/unwind.
        console.log('sleep...');
        sleeps++;
        // Unwinding.
        exports.bysyncify_start_unwind(DATA_ADDR);
        // Fill in the data structure. The first value has the stack location,
        // which for simplicity we can start right after the data structure itself.
        view[DATA_ADDR >> 2] = DATA_ADDR + 8;
        // The end of the stack will not be reached here anyhow.
        view[DATA_ADDR + 4 >> 2] = 1024;
        sleeping = true;
      } else {
        // We are called as part of a resume/rewind. Stop sleeping.
        console.log('resume...');
        exports.bysyncify_stop_rewind();
        // The stack should have been all used up, and so returned to the original state.
        assert(view[DATA_ADDR >> 2] == DATA_ADDR + 8);
        assert(view[DATA_ADDR + 4 >> 2] == 1024);
        sleeping = false;
      }
      logMemory();
    }
  }
});

var exports = instance.exports;
var view = new Int32Array(exports.memory.buffer);

function logMemory() {
  // Log the relevant memory locations for debugging purposes.
  console.log('memory: ', view[0 >> 2], view[4 >> 2], view[8 >> 2], view[12 >> 2], view[16 >> 2], view[20 >> 2], view[24 >> 2]);
}

function runTest(name, expectedSleeps, expectedResult, params) {
  params = params || [];

  console.log('\n==== testing ' + name + ' ====');

  sleeps = 0;

  logMemory();

  // Run until the sleep.
  var result = exports[name].apply(null, params);
  assert(!result, 'results during sleep are meaningless, just 0');
  logMemory();

  for (var i = 0; i < expectedSleeps - 1; i++) {
    console.log('rewind, run until the next sleep');
    exports.bysyncify_start_rewind(DATA_ADDR);
    result = exports[name](); // no need for params on later times
    assert(!result, 'results during sleep are meaningless, just 0');
    assert(!result, 'bad first sleep result');
    logMemory();
  }

  console.log('rewind and run til the end.');
  exports.bysyncify_start_rewind(DATA_ADDR);
  result = exports[name]();
  console.log('final result: ' + result);
  assert(result == expectedResult, 'bad final result');
  logMemory();

  assert(sleeps == expectedSleeps, 'expectedSleeps');
}

//================
// Tests
//================

// A minimal single sleep.
runTest('minimal', 1, 21);

// Two sleeps.
runTest('repeat', 2, 42);

// A value in a local is preserved across a sleep.
runTest('local', 1, 10);

// A local with more operations done on it.
runTest('local2', 1, 22);

// A local with more operations done on it.
runTest('params', 1, 18);
runTest('params', 1, 21, [1, 2]);

console.log('\ntests completed successfully');

