// This script can be customized by setting the following variables in code that
// runs before this script.
//
// The binary to be run. (If not set, we get the filename from argv and read
// from it.)
var binary;
// A second binary to be linked in and run as well. (Can also be read from
// argv.)
var secondBinary;
// Whether we are fuzzing JSPI. In addition to this being set, the "async" and
// "await" keywords must be taken out of the /* KEYWORD */ comments (which they
// are normally in, so as not to affect normal fuzzing).
var JSPI;

// Shell integration: find argv and set up readBinary().
var argv;
var readBinary;
if (typeof process === 'object' && typeof require === 'function') {
  // Node.js.
  argv = process.argv.slice(2);
  readBinary = function(name) {
    var data = require('fs').readFileSync(name);
    if (!data.buffer) data = new Uint8Array(data);
    return data;
  };
} else {
  // A shell like D8.
  if (typeof scriptArgs != 'undefined') {
    argv = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    argv = arguments;
  }
  readBinary = function(name) {
    if (typeof readbuffer === 'function') {
      return new Uint8Array(readbuffer(name));
    } else {
      return read(name, 'binary');
    }
  };
}

if (!binary) {
  binary = readBinary(argv[0]);
}

// Normally we call all the exports of the given wasm file. But, if we are
// passed a final parameter in the form of "exports:X,Y,Z" then we call
// specifically the exports X, Y, and Z.
var exportsToCall;
if (argv.length > 0 && argv[argv.length - 1].startsWith('exports:')) {
  exportsToCall = argv[argv.length - 1].substr('exports:'.length).split(',');
  argv.pop();
}

// If a second parameter is given, it is a second binary that we will link in
// with it.
if (argv[1]) {
  secondBinary = readBinary(argv[1]);
}

// Utilities.
function assert(x, y) {
  if (!x) throw (y || 'assertion failed');// + new Error().stack;
}

// Print out a value in a way that works well for fuzzing.
function printed(x, y) {
  if (typeof y !== 'undefined') {
    // A pair of i32s which are a legalized i64.
    return x + ' ' + y;
  } else if (x === null) {
    // JS has just one null. Print that out rather than typeof null which is
    // 'object', below.
    return 'null';
  } else if (typeof x === 'string') {
    // Emit a string in the same format as the binaryen interpreter. This
    // escaping routine must be kept in sync with String::printEscapedJSON.
    var escaped = '';
    for (u of x) {
      switch (u) {
        case '"':
          escaped += '\\"';
          continue;
        case '\\':
          escaped += '\\\\';
          continue;
        case '\b':
          escaped += '\\b';
          continue;
        case '\f':
          escaped += '\\f';
          continue;
        case '\n':
          escaped += '\\n';
          continue;
        case '\r':
          escaped += '\\r';
          continue;
        case '\t':
          escaped += '\\t';
          continue;
        default:
          break;
      }

      var codePoint = u.codePointAt(0);
      if (32 <= codePoint && codePoint < 127) {
        escaped += u;
        continue
      }

      var printEscape = (codePoint) => {
        escaped += '\\u'
        escaped += ((codePoint & 0xF000) >> 12).toString(16);
        escaped += ((codePoint & 0x0F00) >> 8).toString(16);
        escaped += ((codePoint & 0x00F0) >> 4).toString(16);
        escaped += (codePoint & 0x000F).toString(16);
      };

      if (codePoint < 0x10000) {
        printEscape(codePoint);
      } else {
        printEscape(0xD800 + ((codePoint - 0x10000) >> 10));
        printEscape(0xDC00 + ((codePoint - 0x10000) & 0x3FF));
      }
    }
    return 'string("' + escaped + '")';
  } else if (typeof x === 'bigint') {
    // Print bigints in legalized form, which is two 32-bit numbers of the low
    // and high bits.
    return (Number(x) | 0) + ' ' + (Number(x >> 32n) | 0)
  } else if (typeof x !== 'number') {
    // Something that is not a number or string, like a reference. We can't
    // print a reference because it could look different after opts - imagine
    // that a function gets renamed internally (that is, the problem is that
    // JS printing will emit some info about the reference and not a stable
    // external representation of it). In those cases just print the type,
    // which will be 'object' or 'function'.
    return typeof x;
  } else {
    // A number. Print the whole thing.
    return '' + x;
  }
}

// Fuzzer integration.
function logValue(x, y) {
  console.log('[LoggingExternalInterface logging ' + printed(x, y) + ']');
}

// Track the exports in a map (similar to the Exports object from wasm, i.e.,
// whose keys are strings and whose values are the corresponding exports).
var exports = {};

// Also track exports in a list, to allow access by index. Each entry here will
// be in the form of { name: .., value: .. }. That allows us to log the name of
// the function and also to call it. This is important because different
// functions may have the same name, if they were exported by different
// Instances under the same export names.
var exportList = [];

// Given a wasm function, call it as best we can from JS, and return the result.
function callFunc(func) {
  // Send the function a null for each parameter. Null can be converted without
  // error to both a number and a reference.
  var args = [];
  for (var i = 0; i < func.length; i++) {
    args.push(null);
  }
  return func.apply(null, args);
}

// Calls a given function in a try-catch, swallowing JS exceptions, and return 1
// if we did in fact swallow an exception. Wasm traps are not swallowed (see
// details below).
/* async */ function tryCall(func) {
  try {
    /* await */ func();
    return 0;
  } catch (e) {
    // We only want to catch exceptions, not wasm traps: traps should still
    // halt execution. Handling this requires different code in wasm2js, so
    // check for that first (wasm2js does not define RuntimeError, so use
    // that for the check - when wasm2js is run, we override the entire
    // WebAssembly object with a polyfill, so we know exactly what it
    // contains).
    var wasm2js = !WebAssembly.RuntimeError;
    if (!wasm2js) {
      // When running native wasm, we can detect wasm traps.
      if (e instanceof WebAssembly.RuntimeError) {
        throw e;
      }
    }
    var text = e + '';
    // We must not swallow host limitations here: a host limitation is a
    // problem that means we must not compare the outcome here to any other
    // VM.
    var hostIssues = ['requested new array is too large',
                      'out of memory',
                      'Maximum call stack size exceeded'];
    if (wasm2js) {
      // When wasm2js does trap, it just throws an "abort" error.
      hostIssues.push('abort');
    }
    for (var hostIssue of hostIssues) {
      if (text.includes(hostIssue)) {
        throw e;
      }
    }
    // Otherwise, this is a normal exception we want to catch (a wasm
    // exception, or a conversion error on the wasm/JS boundary, etc.).
    return 1;
  }
}

// Table get/set operations need a BigInt if the table has 64-bit indexes. This
// adds a proper cast as needed.
function toAddressType(table, index) {
  // First, cast to unsigned. We do not support larger indexes anyhow.
  index = index >>> 0;
  if (typeof table.length == 'bigint') {
    return BigInt(index);
  }
  return index;
}

// Set up the imports.
var tempRet0;
var imports = {
  'fuzzing-support': {
    // Logging.
    'log-i32': logValue,
    'log-i64': logValue,
    'log-f32': logValue,
    'log-f64': logValue,
    // JS cannot log v128 values (we trap on the boundary), but we must still
    // provide an import so that we do not trap during linking. (Alternatively,
    // we could avoid running JS on code with SIMD in it, but it is useful to
    // fuzz such code as much as we can.)
    'log-v128': logValue,

    // Throw an exception from JS.
    'throw': () => {
      throw 'some JS error';
    },

    // Table operations.
    'table-get': (index) => {
      return exports.table.get(toAddressType(exports.table, index));
    },
    'table-set': (index, value) => {
      exports.table.set(toAddressType(exports.table, index), value);
    },

    // Export operations.
    'call-export': /* async */ (index) => {
      /* await */ callFunc(exportList[index].value);
    },
    'call-export-catch': /* async */ (index) => {
      return tryCall(/* async */ () => /* await */ callFunc(exportList[index].value));
    },

    // Funcref operations.
    'call-ref': /* async */ (ref) => {
      // This is a direct function reference, and just like an export, it must
      // be wrapped for JSPI.
      ref = wrapExportForJSPI(ref);
      /* await */ callFunc(ref);
    },
    'call-ref-catch': /* async */ (ref) => {
      ref = wrapExportForJSPI(ref);
      return tryCall(/* async */ () => /* await */ callFunc(ref));
    },

    // Sleep a given amount of ms (when JSPI) and return a given id after that.
    'sleep': (ms, id) => {
      if (!JSPI) {
        return id;
      }
      return new Promise((resolve, reject) => {
        setTimeout(() => {
          resolve(id);
        }, 0); // TODO: Use the ms in some reasonable, deterministic manner.
               //       Rather than actually setTimeout on them we could manage
               //       a queue of pending sleeps manually, and order them based
               //       on the "ms" (which would not be literal ms, but just
               //       how many time units to wait).
      });
    },
  },
  // Emscripten support.
  'env': {
    'setTempRet0': function(x) { tempRet0 = x },
    'getTempRet0': function() { return tempRet0 },
  },
};

// If Tags are available, add the import j2wasm expects.
if (typeof WebAssembly.Tag !== 'undefined') {
  imports['imports'] = {
    'j2wasm.ExceptionUtils.tag': new WebAssembly.Tag({
      'parameters': ['externref']
    }),
  };
}

// If JSPI is available, wrap the imports and exports.
if (JSPI) {
  for (var name of ['sleep', 'call-export', 'call-export-catch', 'call-ref',
                    'call-ref-catch']) {
    imports['fuzzing-support'][name] =
      new WebAssembly.Suspending(imports['fuzzing-support'][name]);
  }
}

function wrapExportForJSPI(value) {
  if (JSPI && typeof value === 'function') {
    value = WebAssembly.promising(value);
  }
  return value;
}

// If a second binary will be linked in then set up the imports for
// placeholders. Any import like  (import "placeholder" "0" (func ..  will be
// provided by the secondary module, and must be called using an indirection.
if (secondBinary) {
  imports['placeholder'] = new Proxy({}, {
    get(target, prop, receiver) {
      // Return a function that throws. We could do an indirect call using the
      // exported table, but as we immediately link in the secondary module,
      // these stubs will not be called (they are written to the table, and the
      // secondary module overwrites them). We do need to return something so
      // the primary module links without erroring, though.
      return () => {
        throw 'proxy stub should not be called';
      }
    }
  });
}

// Compile and instantiate a wasm file.
function build(binary) {
  var module = new WebAssembly.Module(binary);

  var instance;
  try {
    instance = new WebAssembly.Instance(module, imports);
  } catch (e) {
    console.log('exception thrown: failed to instantiate module: ' + e);
    quit();
  }

  // Update the exports. Note that this adds onto |exports| and |exportList|,
  // which is intentional: if we build another wasm, or build this one more
  // than once, we want to be able to call them all, so we unify all their
  // exports. (We do trample in |exports| when keys are equal - basically this
  // is a single global namespace - but |exportList| is appended to, so we do
  // keep the ability to call anything that was ever exported.)
  //
  // Note we do not iterate on instance.exports: the order there is not
  // necessarily the order in the wasm (which is what wasm-opt --fuzz-exec uses,
  // for example), because of JS object iteration rules (numbers are considered
  // "array indexes" and appear first,
  // https://tc39.es/ecma262/#sec-ordinaryownpropertykeys).
  for (var e of WebAssembly.Module.exports(module)) {
    var key = e.name;
    var value = instance.exports[key];
    value = wrapExportForJSPI(value);
    exports[key] = value;
    exportList.push({ name: key, value: value });
  }
}

// Simple deterministic hashing, on an unsigned 32-bit seed. See e.g.
// https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
function hashCombine(seed, value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >>> 2);
  return seed >>> 0;
}

// Run the code by calling exports. The optional |ordering| parameter indicates
// howe we should order the calls to the exports: if it is not provided, we call
// them in the natural order, which allows our output to be compared to other
// executions of the wasm (e.g. from wasm-opt --fuzz-exec). If |ordering| is
// provided, it is a random seed we use to make deterministic choices on
// the order of calls.
/* async */ function callExports(ordering) {
  // Call the exports we were told, or if we were not given an explicit list,
  // call them all.
  let relevantExports = exportsToCall || exportList;

  // Build the list of call tasks to run, one for each relevant export.
  let tasks = [];
  for (let e of relevantExports) {
    let name, value;
    if (typeof e === 'string') {
      // We are given a string name to call. Look it up in the global namespace.
      name = e;
      value = exports[e];
    } else {
      // We are given an object form exportList, which has both a name and a
      // value.
      name = e.name;
      value = e.value;
    }

    if (typeof value !== 'function') {
      continue;
    }

    // A task is a name + a function to call. For an export, the function is
    // simply a call of the export.
    tasks.push({ name: name, func: /* async */ () => callFunc(value) });
  }

  // Reverse the array, so the first task is at the end, for efficient
  // popping in the common case.
  tasks.reverse();

  // Execute tasks while they remain.
  while (tasks.length) {
    let task;
    if (ordering === undefined) {
      // Use the natural order.
      task = tasks.pop();
    } else {
      // Pick a random task.
      ordering = hashCombine(ordering, tasks.length);
      let i = ordering % tasks.length;
      task = tasks.splice(i, 1)[0];
    }

    // Execute the task.
    console.log('[fuzz-exec] calling ' + task.name);
    let result;
    try {
      result = task.func();
    } catch (e) {
      console.log('exception thrown: ' + e);
      continue;
    }

    if (JSPI) {
      // When we are changing up the order, in JSPI we can also leave some
      // promises unresolved until later, which lets us interleave them. Note we
      // never defer a task more than once (which would be pointless), and we
      // also only defer a promise (which we check for using .then).
      if (ordering !== undefined && !task.deferred && result &&
          typeof result == 'object' && typeof result.then === 'function') {
        // Hash with -1 here, just to get something different than the hashing a
        // few lines above.
        ordering = hashCombine(ordering, -1);
        if (ordering & 1) {
          // Defer it for later. Reuse the existing task for simplicity.
          console.log(`(jspi: defer ${task.name})`);
          task.func = /* async */ () => {
            console.log(`(jspi: finish ${task.name})`);
            return /* await */ result;
          };
          task.deferred = true;
          tasks.push(task);
          continue;
        }
        // Otherwise, continue down.
      }

      // Await it right now.
      try {
        result = /* await */ result;
      } catch (e) {
        console.log('exception thrown: ' + e);
        continue;
      }
    }

    // Log the result.
    if (typeof result !== 'undefined') {
      console.log('[fuzz-exec] note result: ' + task.name + ' => ' + printed(result));
    }
  }
}

// Build the main wasm.
build(binary);

// Build the second wasm, if one was provided.
if (secondBinary) {
  build(secondBinary);
}

// Run.
callExports();
