#!/usr/bin/python3

'''
Run various fuzzing operations on random inputs, using wasm-opt. See
"testcase_handlers" below for the list of fuzzing operations.

Usage:

./scripts/fuzz_opt.py

That will run forever or until it finds a problem.

You can put files in the local directory 'fuzz' (under the top level of the
binaryen repo) and the fuzzer will treat them as important content to fuzz
with high frequency.

Setup: Some tools are optional, like emcc and wasm2c. The v8 shell (d8),
however, is used in various sub-fuzzers and so it is mandatory.

Note: For afl-fuzz integration, you probably don't want this, and can use
something like

BINARYEN_CORES=1 BINARYEN_PASS_DEBUG=1 afl-fuzz -i afl-testcases/ -o afl-findings/ -m 100 -d -- bin/wasm-opt -ttf --fuzz-exec --Os @@

(that is on a fixed set of arguments to wasm-opt, though - this
script covers different options being passed)
'''

import contextlib
import os
import difflib
import json
import math
import shutil
import subprocess
import random
import re
import sys
import tarfile
import time
import traceback
from os.path import abspath

from test import shared
from test import support


assert sys.version_info.major == 3, 'requires Python 3!'

# parameters

# feature options that are always passed to the tools.
# XXX fp16 is not yet stable, remove from here when it is
CONSTANT_FEATURE_OPTS = ['--all-features', '--disable-fp16']

INPUT_SIZE_MIN = 1024
INPUT_SIZE_MEAN = 40 * 1024
INPUT_SIZE_MAX = 5 * INPUT_SIZE_MEAN

PRINT_WATS = False

given_seed = None

CLOSED_WORLD_FLAG = '--closed-world'


# utilities

def in_binaryen(*args):
    return os.path.join(shared.options.binaryen_root, *args)


def in_bin(tool):
    return os.path.join(shared.options.binaryen_bin, tool)


def random_size():
    if random.random() < 0.25:
        # sometimes do an exponential distribution, which prefers smaller sizes but may
        # also get very high
        ret = int(random.expovariate(1.0 / INPUT_SIZE_MEAN))
        # if the result is valid, use it, otherwise do the normal thing
        # (don't clamp, which would give us a lot of values on the borders)
        if ret >= INPUT_SIZE_MIN and ret <= INPUT_SIZE_MAX:
            return ret

    # most of the time do a simple linear range around the mean
    return random.randint(INPUT_SIZE_MIN, 2 * INPUT_SIZE_MEAN - INPUT_SIZE_MIN)


def make_random_input(input_size, raw_input_data):
    with open(raw_input_data, 'wb') as f:
        f.write(bytes([random.randint(0, 255) for x in range(input_size)]))


def run(cmd, stderr=None, silent=False):
    if not silent:
        print(' '.join(cmd))
    return subprocess.check_output(cmd, stderr=stderr, text=True)


def run_unchecked(cmd):
    print(' '.join(cmd))
    return subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).communicate()[0]


def randomize_pass_debug():
    if random.random() < 0.1:
        print('[pass-debug]')
        os.environ['BINARYEN_PASS_DEBUG'] = '1'
    else:
        os.environ['BINARYEN_PASS_DEBUG'] = '0'
        del os.environ['BINARYEN_PASS_DEBUG']
    print('randomized pass debug:', os.environ.get('BINARYEN_PASS_DEBUG', ''))


@contextlib.contextmanager
def no_pass_debug():
    old_env = os.environ.copy()
    if os.environ.get('BINARYEN_PASS_DEBUG'):
        del os.environ['BINARYEN_PASS_DEBUG']
    try:
        yield
    finally:
        os.environ.update(old_env)


def randomize_feature_opts():
    global FEATURE_OPTS
    FEATURE_OPTS = CONSTANT_FEATURE_OPTS[:]

    if random.random() < 0.1:
        # 10% of the time disable all features, i.e., fuzz the MVP featureset.
        # Fuzzing that is less and less important as more features get enabled
        # by default, but we don't want to lose all coverage for it entirely
        # (and the odds of randomly not selecting any feature, below, is too
        # small - at 17 features it is far less than 1%).
        FEATURE_OPTS += FEATURE_DISABLE_FLAGS
    elif random.random() < 0.333:
        # 1/3 of the remaining 90% pick each feature randomly.
        for possible in FEATURE_DISABLE_FLAGS:
            if random.random() < 0.5:
                FEATURE_OPTS.append(possible)
                if possible in IMPLIED_FEATURE_OPTS:
                    FEATURE_OPTS.extend(IMPLIED_FEATURE_OPTS[possible])
    else:
        # 2/3 of the remaining 90% use them all. This is useful to maximize
        # coverage, as enabling more features enables more optimizations and
        # code paths, and also allows all initial contents to run.

        # The shared-everything feature is new and we want to fuzz it, but it
        # also currently disables fuzzing V8, so disable it most of the time.
        if random.random() < 0.9:
            FEATURE_OPTS.append('--disable-shared-everything')

    print('randomized feature opts:', '\n  ' + '\n  '.join(FEATURE_OPTS))

    # Pick closed or open with equal probability as both matter.
    #
    # Closed world is not a feature flag, technically, since it only makes sense
    # to pass to wasm-opt (and not other tools). But decide on whether we'll
    # be fuzzing in that mode now, as it determinies how we set other things up.
    global CLOSED_WORLD
    CLOSED_WORLD = random.random() < 0.5


ALL_FEATURE_OPTS = ['--all-features', '-all', '--mvp-features', '-mvp']


def update_feature_opts(wasm):
    global FEATURE_OPTS
    # we will re-compute the features; leave all other things as they are
    EXTRA = [x for x in FEATURE_OPTS if not x.startswith('--enable') and
             not x.startswith('--disable') and x not in ALL_FEATURE_OPTS]
    FEATURE_OPTS = run([in_bin('wasm-opt'), wasm] + FEATURE_OPTS + ['--print-features']).strip().split('\n')
    # filter out '', which can happen if no features are enabled
    FEATURE_OPTS = [x for x in FEATURE_OPTS if x]
    print(FEATURE_OPTS, EXTRA)
    FEATURE_OPTS += EXTRA


def randomize_fuzz_settings():
    # a list of the arguments to pass to wasm-opt -ttf when generating the wasm
    global GEN_ARGS
    GEN_ARGS = []

    # a list of the optimizations to run on the wasm
    global FUZZ_OPTS
    FUZZ_OPTS = []

    # a boolean whether NaN values are allowed, or we de-NaN them
    global NANS

    # a boolean whether out of bounds operations are allowed, or we bounds-enforce them
    global OOB

    # a boolean whether we legalize the wasm for JS
    global LEGALIZE

    if random.random() < 0.5:
        NANS = True
    else:
        NANS = False
        GEN_ARGS += ['--denan']
    if random.random() < 0.5:
        OOB = True
    else:
        OOB = False
        GEN_ARGS += ['--no-fuzz-oob']
    if random.random() < 0.5:
        LEGALIZE = True
        GEN_ARGS += ['--legalize-and-prune-js-interface']
    else:
        LEGALIZE = False

    # if GC is enabled then run --dce at the very end, to ensure that our
    # binaries validate in other VMs, due to how non-nullable local validation
    # and unreachable code interact. see
    #   https://github.com/WebAssembly/binaryen/pull/5665
    #   https://github.com/WebAssembly/binaryen/issues/5599
    if '--disable-gc' not in FEATURE_OPTS:
        GEN_ARGS += ['--dce']

        # Add --dce not only when generating the original wasm but to the
        # optimizations we use to create any other wasm file.
        FUZZ_OPTS += ['--dce']

    if CLOSED_WORLD:
        GEN_ARGS += [CLOSED_WORLD_FLAG]
        # Enclose the world much of the time when fuzzing closed-world, so that
        # many types are private and hence optimizable.
        if random.random() < 0.5:
            GEN_ARGS += ['--enclose-world']

    print('randomized settings (NaNs, OOB, legalize):', NANS, OOB, LEGALIZE)


def init_important_initial_contents():
    # Fuzz dir contents are always important to us.
    fuzz_dir = os.path.join(shared.options.binaryen_root, 'fuzz')
    fuzz_cases = shared.get_tests(fuzz_dir, test_suffixes, recursive=True)
    FIXED_IMPORTANT_INITIAL_CONTENTS = fuzz_cases

    # If auto_initial_contents is set we'll also grab all test files that are
    # recent.
    RECENT_DAYS = 30

    # Returns the list of test wast/wat files added or modified within the
    # RECENT_DAYS number of days counting from the commit time of HEAD
    def auto_select_recent_initial_contents():
        # Print 'git log' with changed file status and without commit messages,
        # with commits within RECENT_DAYS number of days, counting from the
        # commit time of HEAD. The reason we use the commit time of HEAD instead
        # of the current system time is to make the results deterministic given
        # the Binaryen HEAD commit.
        from datetime import datetime, timedelta, timezone
        head_ts_str = run(['git', 'log', '-1', '--format=%cd', '--date=raw'],
                          silent=True).split()[0]
        head_dt = datetime.utcfromtimestamp(int(head_ts_str))
        start_dt = head_dt - timedelta(days=RECENT_DAYS)
        start_ts = start_dt.replace(tzinfo=timezone.utc).timestamp()
        log = run(['git', 'log', '--name-status', '--format=', '--date=raw', '--no-renames', f'--since={start_ts}'], silent=True).splitlines()
        # Pick up lines in the form of
        # A       test/../something.wast
        # M       test/../something.wast
        # (wat extension is also included)
        p = re.compile(r'^[AM]\stest' + os.sep + r'(.*\.(wat|wast))$')
        matches = [p.match(e) for e in log]
        auto_set = set([match.group(1) for match in matches if match])
        auto_set = auto_set.difference(set(FIXED_IMPORTANT_INITIAL_CONTENTS))
        return sorted(list(auto_set))

    def is_git_repo():
        try:
            ret = run(['git', 'rev-parse', '--is-inside-work-tree'],
                      silent=True, stderr=subprocess.DEVNULL)
            return ret == 'true\n'
        except subprocess.CalledProcessError:
            return False

    if not is_git_repo() and shared.options.auto_initial_contents:
        print('Warning: The current directory is not a git repository, ' +
              'so not automatically selecting initial contents.')
        shared.options.auto_initial_contents = False

    print('- Important provided initial contents:')
    for test in FIXED_IMPORTANT_INITIAL_CONTENTS:
        print('  ' + test)
    print()

    recent_contents = []
    print('- Recently added or modified initial contents ', end='')
    if shared.options.auto_initial_contents:
        print(f'(automatically selected: within last {RECENT_DAYS} days):')
        recent_contents += auto_select_recent_initial_contents()
    for test in recent_contents:
        print('  ' + test)
    print()

    initial_contents = FIXED_IMPORTANT_INITIAL_CONTENTS + recent_contents
    global IMPORTANT_INITIAL_CONTENTS
    IMPORTANT_INITIAL_CONTENTS = [os.path.join(shared.get_test_dir('.'), t) for t in initial_contents]


INITIAL_CONTENTS_IGNORE = [
    # Float16 is still experimental.
    'f16.wast',
    # not all relaxed SIMD instructions are implemented in the interpreter
    'relaxed-simd.wast',
    # TODO: fuzzer and interpreter support for strings
    'strings.wast',
    'simplify-locals-strings.wast',
    'string-lowering-instructions.wast',
    # TODO: fuzzer and interpreter support for extern conversions
    'extern-conversions.wast',
    # ignore DWARF because it is incompatible with multivalue atm
    'zlib.wasm',
    'cubescript.wasm',
    'class_with_dwarf_noprint.wasm',
    'fib2_dwarf.wasm',
    'fib_nonzero-low-pc_dwarf.wasm',
    'inlined_to_start_dwarf.wasm',
    'fannkuch3_manyopts_dwarf.wasm',
    'fib2_emptylocspan_dwarf.wasm',
    'fannkuch3_dwarf.wasm',
    'dwarf-local-order.wasm',
    'strip-producers.wasm',
    'multi_unit_abbrev_noprint.wasm',
    'reverse_dwarf_abbrevs.wasm',
    'print_g.wasm',
    'print_g_strip-dwarf.wasm',
    'fannkuch0_dwarf.wasm',
    'dwarfdump_roundtrip_dwarfdump.wasm',
    'dwarfdump.wasm',
    'fannkuch3_dwarf.wasm',
    'dwarf-local-order.wasm',
    'dwarf_unit_with_no_abbrevs_noprint.wasm',
    'strip-debug.wasm',
    'multi_line_table_dwarf.wasm',
    'dwarf_with_exceptions.wasm',
    'strip-dwarf.wasm',
    'ignore_missing_func_dwarf.wasm',
    'print.wasm',
    # TODO fuzzer support for multimemory
    'multi-memories-atomics64.wast',
    'multi-memories-basics.wast',
    'multi-memories-simd.wast',
    'multi-memories-atomics64.wasm',
    'multi-memories-basics.wasm',
    'multi-memories-simd.wasm',
    'multi-memories_size.wast',
    # TODO: fuzzer support for internalize/externalize
    'optimize-instructions-gc-extern.wast',
    'gufa-extern.wast',
    # the fuzzer does not support imported memories
    'multi-memory-lowering-import.wast',
    'multi-memory-lowering-import-error.wast',
    # the fuzzer does not support stack switching
    'stack_switching.wast',
    'stack_switching_contnew.wast',
    'stack_switching_contbind.wast',
    'stack_switching_suspend.wast',
    'stack_switching_resume.wast',
    'stack_switching_resume_throw.wast',
    'stack_switching_switch.wast'
]


def pick_initial_contents():
    # if we use an initial wasm file's contents as the basis for the
    # fuzzing, then that filename, or None if we start entirely from scratch
    global INITIAL_CONTENTS

    INITIAL_CONTENTS = None
    # half the time don't use any initial contents
    if random.random() < 0.5:
        return
    # some of the time use initial contents that are known to be especially
    # important
    if IMPORTANT_INITIAL_CONTENTS and random.random() < 0.5:
        test_name = random.choice(IMPORTANT_INITIAL_CONTENTS)
    else:
        test_name = random.choice(all_tests)
    print('initial contents:', test_name)
    if shared.options.auto_initial_contents:
        # when using auto initial contents, we look through the git history to
        # find test files. if a test file was renamed or removed then it may
        # no longer exist, and we should just skip it.
        if not os.path.exists(test_name):
            return
    if os.path.basename(test_name) in INITIAL_CONTENTS_IGNORE:
        return
    assert os.path.exists(test_name)
    # tests that check validation errors are not helpful for us
    if '.fail.' in test_name:
        print('initial contents is just a .fail test')
        return
    if os.path.basename(test_name) in [
        # contains too many segments to run in a wasm VM
        'limit-segments_disable-bulk-memory.wast',
        # https://github.com/WebAssembly/binaryen/issues/3203
        'simd.wast',
        # corner cases of escaping of names is not interesting
        'names.wast',
        # huge amount of locals that make it extremely slow
        'too_much_for_liveness.wasm'
    ]:
        print('initial contents is disallowed')
        return

    if test_name.endswith('.wast'):
        # this can contain multiple modules, pick one
        split_parts = support.split_wast(test_name)
        if len(split_parts) > 1:
            index = random.randint(0, len(split_parts) - 1)
            chosen = split_parts[index]
            module, asserts = chosen
            if not module:
                # there is no module in this choice (just asserts), ignore it
                print('initial contents has no module')
                return
            test_name = abspath('initial.wat')
            with open(test_name, 'w') as f:
                f.write(module)
            print('  picked submodule %d from multi-module wast' % index)

    global FEATURE_OPTS
    FEATURE_OPTS += [
        # avoid multivalue for now due to bad interactions with gc non-nullable
        # locals in stacky code. for example, this fails to roundtrip as the
        # tuple code ends up creating stacky binary code that needs to spill
        # non-nullable references to locals, which is not allowed:
        #
        # (module
        #  (type $other (struct))
        #  (func $foo (result (ref $other))
        #   (select
        #    (struct.new $other)
        #    (struct.new $other)
        #    (tuple.extract 2 1
        #     (tuple.make 2
        #      (i32.const 0)
        #      (i32.const 0)
        #     )
        #    )
        #   )
        #  )
        # )
        '--disable-multivalue',
    ]

    # the given wasm may not work with the chosen feature opts. for example, if
    # we pick atomics.wast but want to run with --disable-atomics, then we'd
    # error, so we need to test the wasm. first, make sure it doesn't have a
    # features section, as that would enable a feature that we might want to
    # be disabled, and our test would not error as we want it to.
    if test_name.endswith('.wasm'):
        temp_test_name = 'initial.wasm'
        try:
            run([in_bin('wasm-opt'), test_name, '-all', '--strip-target-features',
                 '-o', temp_test_name])
        except Exception:
            # the input can be invalid if e.g. it is raw data that is used with
            # -ttf as fuzzer input
            print('(initial contents are not valid wasm, ignoring)')
            return
        test_name = temp_test_name

    # Next, test the wasm. Note that we must check for closed world explicitly
    # here, as a testcase may only work in an open world, which means we need to
    # skip it.
    args = FEATURE_OPTS
    if CLOSED_WORLD:
        args.append(CLOSED_WORLD_FLAG)
    try:
        run([in_bin('wasm-opt'), test_name] + args,
            stderr=subprocess.PIPE,
            silent=True)
    except Exception:
        print('(initial contents not valid for features, ignoring)')
        return

    INITIAL_CONTENTS = test_name


# Test outputs we want to ignore are marked this way.
IGNORE = '[binaryen-fuzzer-ignore]'

# Traps are reported as [trap REASON]
TRAP_PREFIX = '[trap '

# Host limits are reported as [host limit REASON]
HOST_LIMIT_PREFIX = '[host limit '

# --fuzz-exec reports calls as [fuzz-exec] calling foo
FUZZ_EXEC_CALL_PREFIX = '[fuzz-exec] calling'

# --fuzz-exec reports a stack limit using this notation
STACK_LIMIT = '[trap stack limit]'

# V8 reports this error in rare cases due to limitations in our handling of non-
# nullable locals in unreachable code, see
#   https://github.com/WebAssembly/binaryen/pull/5665
#   https://github.com/WebAssembly/binaryen/issues/5599
# and also see the --dce workaround below that also links to those issues.
V8_UNINITIALIZED_NONDEF_LOCAL = 'uninitialized non-defaultable local'

# JS exceptions are logged as exception thrown: REASON
EXCEPTION_PREFIX = 'exception thrown: '


# given a call line that includes FUZZ_EXEC_CALL_PREFIX, return the export that
# is called
def get_export_from_call_line(call_line):
    assert FUZZ_EXEC_CALL_PREFIX in call_line
    return call_line.split(FUZZ_EXEC_CALL_PREFIX)[1].strip()


# compare two strings, strictly
def compare(x, y, context, verbose=True):
    if x != y and x != IGNORE and y != IGNORE:
        message = ''.join([a + '\n' for a in difflib.unified_diff(x.splitlines(), y.splitlines(), fromfile='expected', tofile='actual')])
        if verbose:
            raise Exception(context + " comparison error, expected to have '%s' == '%s', diff:\n\n%s" % (
                x, y,
                message
            ))
        else:
            raise Exception(context + "\nDiff:\n\n%s" % (message))


# converts a possibly-signed integer to an unsigned integer
def unsign(x, bits):
    return x & ((1 << bits) - 1)


# numbers are "close enough" if they just differ in printing, as different
# vms may print at different precision levels and verbosity
def numbers_are_close_enough(x, y):
    # handle nan comparisons like -nan:0x7ffff0 vs NaN, ignoring the bits
    if 'nan' in x.lower() and 'nan' in y.lower():
        return True
    # if one input is a pair, then it is in fact a 64-bit integer that is
    # reported as two 32-bit chunks. convert such 'low high' pairs into a 64-bit
    # integer for comparison to the other value
    if ' ' in x or ' ' in y:
        def to_64_bit(a):
            if ' ' not in a:
                return unsign(int(a), bits=64)
            low, high = a.split(' ')
            return unsign(int(low), 32) + (1 << 32) * unsign(int(high), 32)

        return to_64_bit(x) == to_64_bit(y)
    # float() on the strings will handle many minor differences, like
    # float('1.0') == float('1') , float('inf') == float('Infinity'), etc.
    try:
        return float(x) == float(y)
    except Exception:
        pass
    # otherwise, try a full eval which can handle i64s too
    try:
        ex = eval(x)
        ey = eval(y)
        return ex == ey or float(ex) == float(ey)
    except Exception as e:
        print('failed to check if numbers are close enough:', e)
        return False


FUZZ_EXEC_NOTE_RESULT = '[fuzz-exec] note result'


# compare between vms, which may slightly change how numbers are printed
def compare_between_vms(x, y, context):
    x_lines = x.splitlines()
    y_lines = y.splitlines()
    if len(x_lines) != len(y_lines):
        return compare(x, y, context + ' (note: different number of lines between vms)')

    num_lines = len(x_lines)
    for i in range(num_lines):
        x_line = x_lines[i]
        y_line = y_lines[i]
        if x_line != y_line:
            # this is different, but maybe it's a vm difference we can ignore
            LEI_LOGGING = '[LoggingExternalInterface logging'
            if x_line.startswith(LEI_LOGGING) and y_line.startswith(LEI_LOGGING):
                x_val = x_line[len(LEI_LOGGING) + 1:-1]
                y_val = y_line[len(LEI_LOGGING) + 1:-1]
                if numbers_are_close_enough(x_val, y_val):
                    continue
            if x_line.startswith(FUZZ_EXEC_NOTE_RESULT) and y_line.startswith(FUZZ_EXEC_NOTE_RESULT):
                x_val = x_line.split(' ')[-1]
                y_val = y_line.split(' ')[-1]
                if numbers_are_close_enough(x_val, y_val):
                    continue

            # this failed to compare. print a custom diff of the relevant lines
            MARGIN = 3
            start = max(i - MARGIN, 0)
            end = min(i + MARGIN, num_lines)
            return compare('\n'.join(x_lines[start:end]), '\n'.join(y_lines[start:end]), context)


def fix_output(out):
    # large doubles may print slightly different on different VMs
    def fix_double(x):
        x = x.group(1)
        if 'nan' in x or 'NaN' in x:
            x = 'nan'
        else:
            x = x.replace('Infinity', 'inf')
            x = str(float(x))
        return 'f64.const ' + x
    out = re.sub(r'f64\.const (-?[nanN:abcdefxIity\d+-.]+)', fix_double, out)

    # mark traps from wasm-opt as exceptions, even though they didn't run in a vm
    out = out.replace(TRAP_PREFIX, EXCEPTION_PREFIX + TRAP_PREFIX)

    # funcref(0) has the index of the function in it, and optimizations can
    # change that index, so ignore it
    out = re.sub(r'funcref\([\d\w$+-_:]+\)', 'funcref()', out)

    # JS prints i31 as just a number, so change "i31ref(N)" (which C++ emits)
    # to "N".
    out = re.sub(r'i31ref\((-?\d+)\)', r'\1', out)

    # Tag names may change due to opts, so canonicalize them.
    out = re.sub(r' tag\$\d+', ' tag', out)

    lines = out.splitlines()
    for i in range(len(lines)):
        line = lines[i]
        if 'Warning: unknown flag' in line or 'Try --help for options' in line:
            # ignore some VM warnings that don't matter, like if a newer V8 has
            # removed a flag that is no longer needed. but print the line so the
            # developer can see it.
            print(line)
            lines[i] = None
        elif EXCEPTION_PREFIX in line:
            # exceptions may differ when optimizing, but an exception should
            # occur, so ignore their types (also js engines print them out
            # slightly differently)
            lines[i] = '     *exception*'
    return '\n'.join([line for line in lines if line is not None])


def fix_spec_output(out):
    out = fix_output(out)
    # spec shows a pointer when it traps, remove that
    out = '\n'.join(map(lambda x: x if 'runtime trap' not in x else x[x.find('runtime trap'):], out.splitlines()))
    # https://github.com/WebAssembly/spec/issues/543 , float consts are messed up
    out = '\n'.join(map(lambda x: x if 'f32' not in x and 'f64' not in x else '', out.splitlines()))
    return out


ignored_vm_runs = 0
ignored_vm_run_reasons = dict()


# Notes a VM run that we ignore, and the reason for it (for metrics purposes).
# Extra text can also be printed that is not included in the metrics.
def note_ignored_vm_run(reason, extra_text='', amount=1):
    global ignored_vm_runs
    print(f'(ignore VM run: {reason}{extra_text})')
    ignored_vm_runs += amount
    ignored_vm_run_reasons.setdefault(reason, 0)
    ignored_vm_run_reasons[reason] += amount


def run_vm(cmd):
    def filter_known_issues(output):
        known_issues = [
            # can be caused by flatten, ssa, etc. passes
            'local count too large',
            # can be caused by (array.new $type -1) etc.
            'requested new array is too large',
            # https://github.com/WebAssembly/binaryen/issues/3767
            # note that this text is a little too broad, but the problem is rare
            # enough that it's unlikely to hide an unrelated issue
            'found br_if of type',
            # this text is emitted from V8 when it runs out of memory during a
            # GC allocation.
            'out of memory',
            # if the call stack is exceeded we must ignore this, as
            # optimizations can change whether this happens or not (e.g. by
            # removing locals, which makes stack frames smaller), which is
            # noticeable.
            'Maximum call stack size exceeded',
            # all host limitations are arbitrary and may differ between VMs and
            # also be affected by optimizations, so ignore them.
            # this is the prefix that the binaryen interpreter emits. For V8,
            # there is no single host-limit signal, and we have the earlier
            # strings in this list for known issues (to which more need to be
            # added as necessary).
            HOST_LIMIT_PREFIX,
            # see comment above on this constant
            V8_UNINITIALIZED_NONDEF_LOCAL,
            # V8 does not accept nullable stringviews
            # (https://github.com/WebAssembly/binaryen/pull/6574)
            'expected (ref stringview_wtf16), got nullref',
            'expected type (ref stringview_wtf16), found ref.null of type nullref',
            # wasm64 memories have a V8 limit
            'larger than implementation limit',
        ]
        for issue in known_issues:
            if issue in output:
                note_ignored_vm_run(issue)
                return IGNORE
        return output

    try:
        # some known issues do not cause the entire process to fail
        return filter_known_issues(run(cmd))
    except subprocess.CalledProcessError:
        # other known issues do make it fail, so re-run without checking for
        # success and see if we should ignore it
        if filter_known_issues(run_unchecked(cmd)) == IGNORE:
            return IGNORE
        raise


MAX_INTERPRETER_ENV_VAR = 'BINARYEN_MAX_INTERPRETER_DEPTH'
MAX_INTERPRETER_DEPTH = 1000


def run_bynterp(wasm, args):
    # increase the interpreter stack depth, to test more things
    os.environ[MAX_INTERPRETER_ENV_VAR] = str(MAX_INTERPRETER_DEPTH)
    try:
        return run_vm([in_bin('wasm-opt'), wasm] + FEATURE_OPTS + args)
    finally:
        del os.environ['BINARYEN_MAX_INTERPRETER_DEPTH']


# Enable even more staged things than V8_OPTS. V8_OPTS are the flags we want to
# use when testing, and enable all features we test against, while --future may
# also enable non-feature things like new JITs and such (which are never needed
# for our normal tests, but do make sense to fuzz for V8's sake). We do this
# randomly for more variety.
def get_v8_extra_flags():
    return ['--future'] if random.random() < 0.5 else []


V8_LIFTOFF_ARGS = ['--liftoff', '--no-wasm-tier-up']


# Default to running with liftoff enabled, because we need to pick either
# liftoff or turbo* for consistency (otherwise running the same command twice
# may have different results due to NaN nondeterminism), and liftoff is faster
# for small things.
def run_d8_js(js, args=[], liftoff=True):
    cmd = [shared.V8] + shared.V8_OPTS
    cmd += get_v8_extra_flags()
    if liftoff:
        cmd += V8_LIFTOFF_ARGS
    cmd += [js]
    if args:
        cmd += ['--'] + args
    return run_vm(cmd)


FUZZ_SHELL_JS = in_binaryen('scripts', 'fuzz_shell.js')


def run_d8_wasm(wasm, liftoff=True, args=[]):
    return run_d8_js(FUZZ_SHELL_JS, [wasm] + args, liftoff=liftoff)


def all_disallowed(features):
    return not any(('--enable-' + x) in FEATURE_OPTS for x in features)


class TestCaseHandler:
    # how frequent this handler will be run. 1 means always run it, 0.5 means half the
    # time
    frequency = 1

    def __init__(self):
        self.num_runs = 0

    # If the core handle_pair() method is not overridden, it calls handle() on
    # each of the items. That is useful if you just want the two wasms and don't
    # care about their relationship.
    def handle_pair(self, input, before_wasm, after_wasm, opts):
        self.handle(before_wasm)
        self.handle(after_wasm)

    def can_run_on_wasm(self, wasm):
        return True

    def increment_runs(self):
        self.num_runs += 1

    def count_runs(self):
        return self.num_runs


# Fuzz the interpreter with --fuzz-exec.
class FuzzExec(TestCaseHandler):
    frequency = 1

    def handle_pair(self, input, before_wasm, after_wasm, opts):
        run([in_bin('wasm-opt'), before_wasm] + opts + ['--fuzz-exec'])


class CompareVMs(TestCaseHandler):
    frequency = 1

    def __init__(self):
        super(CompareVMs, self).__init__()

        class BinaryenInterpreter:
            name = 'binaryen interpreter'

            def run(self, wasm):
                output = run_bynterp(wasm, ['--fuzz-exec-before'])
                if output != IGNORE:
                    calls = output.count(FUZZ_EXEC_CALL_PREFIX)
                    errors = output.count(TRAP_PREFIX) + output.count(HOST_LIMIT_PREFIX)
                    if errors > calls / 2:
                        # A significant amount of execution on this testcase
                        # simply trapped, and was not very useful, so mark it
                        # as ignored. Ideally the fuzzer testcases would be
                        # improved to reduce this number.
                        #
                        # Note that we don't change output=IGNORE as there may
                        # still be useful testing here (up to 50%), so we only
                        # note that this is a mostly-ignored run, but we do not
                        # ignore the parts that are useful.
                        #
                        # Note that we set amount to 0.5 because we are run both
                        # on the before wasm and the after wasm. Those will be
                        # in sync (because the optimizer does not remove traps)
                        # and so by setting 0.5 we only increment by 1 for the
                        # entire iteration.
                        note_ignored_vm_run('too many errors vs calls',
                                            extra_text=f' ({calls} calls, {errors} errors)',
                                            amount=0.5)
                return output

            def can_run(self, wasm):
                return True

            def can_compare_to_self(self):
                return True

            def can_compare_to_others(self):
                return True

        class D8:
            name = 'd8'

            def run(self, wasm, extra_d8_flags=[]):
                return run_vm([shared.V8, FUZZ_SHELL_JS] + shared.V8_OPTS + get_v8_extra_flags() + extra_d8_flags + ['--', wasm])

            def can_run(self, wasm):
                # V8 does not support shared memories when running with
                # shared-everything enabled, so do not fuzz shared-everything
                # for now.
                return all_disallowed(['shared-everything'])

            def can_compare_to_self(self):
                # With nans, VM differences can confuse us, so only very simple VMs
                # can compare to themselves after opts in that case.
                return not NANS

            def can_compare_to_others(self):
                # If not legalized, the JS will fail immediately, so no point to
                # compare to others.
                return LEGALIZE and not NANS

        class D8Liftoff(D8):
            name = 'd8_liftoff'

            def run(self, wasm):
                return super(D8Liftoff, self).run(wasm, extra_d8_flags=V8_LIFTOFF_ARGS)

        class D8Turboshaft(D8):
            name = 'd8_turboshaft'

            def run(self, wasm):
                return super(D8Turboshaft, self).run(wasm, extra_d8_flags=['--no-liftoff', '--turboshaft-wasm', '--turboshaft-wasm-instruction-selection-staged'])

        class Wasm2C:
            name = 'wasm2c'

            def __init__(self):
                # look for wabt in the path. if it's not here, don't run wasm2c
                try:
                    wabt_bin = shared.which('wasm2c')
                    wabt_root = os.path.dirname(os.path.dirname(wabt_bin))
                    self.wasm2c_dir = os.path.join(wabt_root, 'wasm2c')
                    if not os.path.isdir(self.wasm2c_dir):
                        print('wabt found, but not wasm2c support dir')
                        self.wasm2c_dir = None
                except Exception as e:
                    print('warning: no wabt found:', e)
                    self.wasm2c_dir = None

            def can_run(self, wasm):
                if self.wasm2c_dir is None:
                    return False
                # if we legalize for JS, the ABI is not what C wants
                if LEGALIZE:
                    return False
                # relatively slow, so run it less frequently
                if random.random() < 0.5:
                    return False
                # wasm2c doesn't support most features
                return all_disallowed(['exception-handling', 'simd', 'threads', 'bulk-memory', 'nontrapping-float-to-int', 'tail-call', 'sign-ext', 'reference-types', 'multivalue', 'gc'])

            def run(self, wasm):
                run([in_bin('wasm-opt'), wasm, '--emit-wasm2c-wrapper=main.c'] + FEATURE_OPTS)
                run(['wasm2c', wasm, '-o', 'wasm.c'])
                compile_cmd = ['clang', 'main.c', 'wasm.c', os.path.join(self.wasm2c_dir, 'wasm-rt-impl.c'), '-I' + self.wasm2c_dir, '-lm', '-Werror']
                run(compile_cmd)
                return run_vm(['./a.out'])

            def can_compare_to_self(self):
                # The binaryen optimizer changes NaNs in the ways that wasm
                # expects, but that's not quite what C has
                return not NANS

            def can_compare_to_others(self):
                # C won't trap on OOB, and NaNs can differ from wasm VMs
                return not OOB and not NANS

        class Wasm2C2Wasm(Wasm2C):
            name = 'wasm2c2wasm'

            def __init__(self):
                super(Wasm2C2Wasm, self).__init__()

                self.has_emcc = shared.which('emcc') is not None

            def run(self, wasm):
                run([in_bin('wasm-opt'), wasm, '--emit-wasm2c-wrapper=main.c'] + FEATURE_OPTS)
                run(['wasm2c', wasm, '-o', 'wasm.c'])
                compile_cmd = ['emcc', 'main.c', 'wasm.c',
                               os.path.join(self.wasm2c_dir, 'wasm-rt-impl.c'),
                               '-I' + self.wasm2c_dir,
                               '-lm',
                               '-s', 'ENVIRONMENT=shell',
                               '-s', 'ALLOW_MEMORY_GROWTH']
                # disable the signal handler: emcc looks like unix, but wasm has
                # no signals
                compile_cmd += ['-DWASM_RT_MEMCHECK_SIGNAL_HANDLER=0']
                if random.random() < 0.5:
                    compile_cmd += ['-O' + str(random.randint(1, 3))]
                elif random.random() < 0.5:
                    if random.random() < 0.5:
                        compile_cmd += ['-Os']
                    else:
                        compile_cmd += ['-Oz']
                # avoid pass-debug on the emcc invocation itself (which runs
                # binaryen to optimize the wasm), as the wasm here can be very
                # large and it isn't what we are focused on testing here
                with no_pass_debug():
                    run(compile_cmd)
                return run_d8_js(abspath('a.out.js'))

            def can_run(self, wasm):
                # quite slow (more steps), so run it less frequently
                if random.random() < 0.8:
                    return False
                # prefer not to run if the wasm is very large, as it can OOM
                # the JS engine.
                return super(Wasm2C2Wasm, self).can_run(wasm) and self.has_emcc and \
                    os.path.getsize(wasm) <= INPUT_SIZE_MEAN

            def can_compare_to_others(self):
                # NaNs can differ from wasm VMs
                return not NANS

        # the binaryen interpreter is specifically useful for various things
        self.bynterpreter = BinaryenInterpreter()

        self.vms = [self.bynterpreter,
                    D8(),
                    D8Liftoff(),
                    D8Turboshaft(),
                    # FIXME: Temprorary disable. See issue #4741 for more details
                    # Wasm2C(),
                    # Wasm2C2Wasm()
                    ]

    def handle_pair(self, input, before_wasm, after_wasm, opts):
        global ignored_vm_runs

        before = self.run_vms(before_wasm)

        after = self.run_vms(after_wasm)
        self.compare_before_and_after(before, after)

    def run_vms(self, wasm):
        ignored_before = ignored_vm_runs

        # vm_results will map vms to their results
        vm_results = {}
        for vm in self.vms:
            if vm.can_run(wasm):
                print(f'[CompareVMs] running {vm.name}')
                vm_results[vm] = fix_output(vm.run(wasm))

                # If the binaryen interpreter hit a host limitation then do not
                # run other VMs, as that is risky: the host limitation may be an
                # an OOM which could be very costly (lots of swapping, and the
                # OOM may change after opts that remove allocations etc.), or it
                # might be an atomic wait which other VMs implement fully (and
                # the wait might be very long). In general host limitations
                # should be rare (which can be verified by looking at the
                # details of how many things we ended up ignoring), and when we
                # see one we are in a situation that we can't fuzz properly.
                if vm == self.bynterpreter and vm_results[vm] == IGNORE:
                    print('(ignored, so not running other VMs)')

                    # the ignoring should have been noted during run_vms()
                    assert(ignored_vm_runs > ignored_before)

                    return vm_results

        # compare between the vms on this specific input
        first_vm = None
        for vm in vm_results.keys():
            if vm.can_compare_to_others():
                if first_vm is None:
                    first_vm = vm
                else:
                    compare_between_vms(vm_results[first_vm], vm_results[vm], 'CompareVMs between VMs: ' + first_vm.name + ' and ' + vm.name)

        return vm_results

    def compare_before_and_after(self, before, after):
        # compare each VM to itself on the before and after inputs
        for vm in before.keys():
            if vm in after and vm.can_compare_to_self():
                compare(before[vm], after[vm], 'CompareVMs between before and after: ' + vm.name)


# Check for determinism - the same command must have the same output.
class CheckDeterminism(TestCaseHandler):
    frequency = 0.2

    def handle_pair(self, input, before_wasm, after_wasm, opts):
        # check for determinism
        run([in_bin('wasm-opt'), before_wasm, '-o', abspath('b1.wasm')] + opts)
        run([in_bin('wasm-opt'), before_wasm, '-o', abspath('b2.wasm')] + opts)
        b1 = open('b1.wasm', 'rb').read()
        b2 = open('b2.wasm', 'rb').read()
        if (b1 != b2):
            run([in_bin('wasm-dis'), abspath('b1.wasm'), '-o', abspath('b1.wat')] + FEATURE_OPTS)
            run([in_bin('wasm-dis'), abspath('b2.wasm'), '-o', abspath('b2.wat')] + FEATURE_OPTS)
            t1 = open(abspath('b1.wat'), 'r').read()
            t2 = open(abspath('b2.wat'), 'r').read()
            compare(t1, t2, 'Output must be deterministic.', verbose=False)


class Wasm2JS(TestCaseHandler):
    frequency = 0.1

    def handle_pair(self, input, before_wasm, after_wasm, opts):
        before_wasm_temp = before_wasm + '.temp.wasm'
        after_wasm_temp = after_wasm + '.temp.wasm'
        # legalize the before wasm, so that comparisons to the interpreter
        # later make sense (if we don't do this, the wasm may have i64 exports).
        # after applying other necessary fixes, we'll recreate the after wasm
        # from scratch.
        run([in_bin('wasm-opt'), before_wasm, '--legalize-and-prune-js-interface', '-o', before_wasm_temp] + FEATURE_OPTS)
        compare_before_to_after = random.random() < 0.5
        compare_to_interpreter = compare_before_to_after and random.random() < 0.5
        if compare_before_to_after:
            # to compare the wasm before and after optimizations, we must
            # remove operations that wasm2js does not support with full
            # precision, such as i64-to-f32, as the optimizer can give different
            # results.
            simplification_passes = ['--stub-unsupported-js']
            if compare_to_interpreter:
                # unexpectedly-unaligned loads/stores work fine in wasm in general but
                # not in wasm2js, since typed arrays silently round down, effectively.
                # if we want to compare to the interpreter, remove unaligned
                # operations (by forcing alignment 1, then lowering those into aligned
                # components, which means all loads and stores are of a single byte).
                simplification_passes += ['--dealign', '--alignment-lowering']
            run([in_bin('wasm-opt'), before_wasm_temp, '-o', before_wasm_temp] + simplification_passes + FEATURE_OPTS)
        # now that the before wasm is fixed up, generate a proper after wasm
        run([in_bin('wasm-opt'), before_wasm_temp, '-o', after_wasm_temp] + opts + FEATURE_OPTS)
        # always check for compiler crashes
        before = self.run(before_wasm_temp)
        after = self.run(after_wasm_temp)
        if NANS:
            # with NaNs we can't compare the output, as a reinterpret through
            # memory might end up different in JS than wasm
            return
        # we also cannot compare if the wasm hits a trap, as wasm2js does not
        # trap on many things wasm would, and in those cases it can do weird
        # undefined things. in such a case, at least compare up until before
        # the trap, which lets us compare at least some results in some cases.
        # (this is why wasm2js is not in CompareVMs, which does full
        # comparisons - we need to limit the comparison in a special way here)
        interpreter = run_bynterp(before_wasm_temp, ['--fuzz-exec-before'])
        if TRAP_PREFIX in interpreter:
            trap_index = interpreter.index(TRAP_PREFIX)
            # we can't test this function, which the trap is in the middle of.
            # erase everything from this function's output and onward, so we
            # only compare the previous trap-free code
            call_start = interpreter.rindex(FUZZ_EXEC_CALL_PREFIX, 0, trap_index)
            call_end = interpreter.index('\n', call_start)
            call_line = interpreter[call_start:call_end]
            before = before[:before.index(call_line)]
            after = after[:after.index(call_line)]
            interpreter = interpreter[:interpreter.index(call_line)]

        def fix_output_for_js(x):
            # start with the normal output fixes that all VMs need
            x = fix_output(x)

            # replace null with 0. the fuzzing harness passes in nulls instead
            # the specific type of a parameter (since null can be cast to
            # anything without issue, and all fuzz_shell.js knows on the JS side
            # is the number of parameters), which can be noticeable in a
            # situation where we optimize and remove casts, like here:
            #
            # function foo(x) { return x | 0; }
            #
            # When optimizing we can remove that | 0, which is valid if the
            # input is valid, but as we said, the fuzz harness passes in a value
            # of the wrong type - which would be cast on use, but if we remove
            # the casts, we end up returning null here and not 0, which the
            # fuzzer can notice.
            x = re.sub(r' null', ' 0', x)

            # check if a number is 0 or a subnormal, which is basically zero
            def is_basically_zero(x):
                # to check if something is a subnormal, compare it to the largest one
                return x >= 0 and x <= 2.22507385850720088902e-308

            def fix_number(x):
                x = x.group(1)
                try:
                    x = float(x)
                    # There appear to be some cases where JS VMs will print
                    # subnormals in full detail while other VMs do not, and vice
                    # versa. Ignore such really tiny numbers.
                    if is_basically_zero(x):
                        x = 0
                except ValueError:
                    # not a floating-point number, nothing to do
                    pass
                return ' => ' + str(x)

            # logging notation is "function_name => result", look for that with
            # a floating-point result that may need to be fixed up
            return re.sub(r' => (-?[\d+-.e\-+]+)', fix_number, x)

        before = fix_output_for_js(before)
        after = fix_output_for_js(after)
        if compare_before_to_after:
            compare_between_vms(before, after, 'Wasm2JS (before/after)')
            if compare_to_interpreter:
                interpreter = fix_output_for_js(interpreter)
                compare_between_vms(before, interpreter, 'Wasm2JS (vs interpreter)')

    def run(self, wasm):
        with open(FUZZ_SHELL_JS) as f:
            wrapper = f.read()
        cmd = [in_bin('wasm2js'), wasm, '--emscripten']
        # avoid optimizations if we have nans, as we don't handle them with
        # full precision and optimizations can change things
        # OOB accesses are also an issue with optimizations, that can turn the
        # loaded "undefined" into either 0 (with an |0) or stay undefined
        # in optimized code.
        if not NANS and not OOB and random.random() < 0.5:
            # when optimizing also enable deterministic mode, to avoid things
            # like integer divide by zero causing false positives (1 / 0 is
            # Infinity without a  | 0 , and 0 with one, and the truthiness of
            # those differs; we don't want to care about this because it
            # would trap in wasm anyhow)
            cmd += ['-O', '--deterministic']
        main = run(cmd + FEATURE_OPTS)
        with open(os.path.join(shared.options.binaryen_root, 'scripts', 'wasm2js.js')) as f:
            glue = f.read()
        js_file = wasm + '.js'
        with open(js_file, 'w') as f:
            f.write(glue)
            f.write(main)
            f.write(wrapper)
        return run_vm([shared.NODEJS, js_file, abspath('a.wasm')])

    def can_run_on_wasm(self, wasm):
        # TODO: properly handle memory growth. right now the wasm2js handler
        # uses --emscripten which assumes the Memory is created before, and
        # wasm2js.js just starts with a size of 1 and no limit. We should switch
        # to non-emscripten mode or adding memory information, or check
        # specifically for growth here
        if INITIAL_CONTENTS:
            return False
        return all_disallowed(['exception-handling', 'simd', 'threads', 'bulk-memory', 'nontrapping-float-to-int', 'tail-call', 'sign-ext', 'reference-types', 'multivalue', 'gc', 'multimemory', 'memory64'])


# given a wasm, find all the exports of particular kinds (for example, kinds
# can be ['func', 'table'] and then we would find exported functions and
# tables).
def get_exports(wasm, kinds):
    wat = run([in_bin('wasm-dis'), wasm] + FEATURE_OPTS)
    p = re.compile(r'^ [(]export "(.*[^\\]?)" [(](?:' + '|'.join(kinds) + ')')
    exports = []
    for line in wat.splitlines():
        m = p.match(line)
        if m:
            export = m[1]
            exports.append(export)
    return exports


# given a wasm and a list of exports we want to keep, remove all other exports.
# we also keep a list of default exports, unless that is overridden (overriding
# it may lead to changes in behavior).
def filter_exports(wasm, output, keep, keep_defaults=True):
    # based on
    # https://github.com/WebAssembly/binaryen/wiki/Pruning-unneeded-code-in-wasm-files-with-wasm-metadce#example-pruning-exports

    # we append to keep; avoid modifying the object that was sent in.
    keep = keep[:]

    if keep_defaults:
        # some exports must normally be preserved, if they exist, like the table
        # (which can be called from JS imports for table operations).
        existing_exports = set(get_exports(wasm, ['func', 'table']))
        for export in ['table']:
            if export in existing_exports:
                keep.append(export)

    # build json to represent the exports we want.
    graph = [{
        'name': 'outside',
        'reaches': [f'export-{export}' for export in keep],
        'root': True
    }]
    for export in keep:
        graph.append({
            'name': f'export-{export}',
            'export': export
        })

    with open('graph.json', 'w') as f:
        f.write(json.dumps(graph))

    # prune the exports
    run([in_bin('wasm-metadce'), wasm, '-o', output, '--graph-file', 'graph.json'] + FEATURE_OPTS)


# Check if a wasm file would notice changes to exports. Normally removing an
# export that is not called, for example, would not be observable, but if the
# "call-export*" functions are present then such changes can break us.
def wasm_notices_export_changes(wasm):
    # we could be more precise here and disassemble the wasm to look for an
    # actual import with name "call-export*", but looking for the string should
    # have practically no false positives.
    return b'call-export' in open(wasm, 'rb').read()


# Fuzz the interpreter with --fuzz-exec -tnh. The tricky thing with traps-never-
# happen mode is that if a trap *does* happen then that is undefined behavior,
# and the optimizer was free to make changes to observable behavior there. The
# fuzzer therefore needs to ignore code that traps.
class TrapsNeverHappen(TestCaseHandler):
    frequency = 0.25

    def handle_pair(self, input, before_wasm, after_wasm, opts):
        before = run_bynterp(before_wasm, ['--fuzz-exec-before'])

        if before == IGNORE:
            # There is no point to continue since we can't compare this output
            # to anything, and there is a risk since if we did so we might run
            # into an infinite loop (see below).
            return

        # if a trap happened, we must stop comparing from that.
        if TRAP_PREFIX in before:
            trap_index = before.index(TRAP_PREFIX)
            # we can't test this function, which the trap is in the middle of
            # (tnh could move the trap around, so even things before the trap
            # are unsafe). we can only safely call exports before this one, so
            # remove those from the binary.
            #
            # first, find the function call during which the trap happened, by
            # finding the call line right before us. that is, the output looks
            # like this:
            #
            #   [fuzz-exec] calling foo
            #   .. stuff happening during foo ..
            #   [fuzz-exec] calling bar
            #   .. stuff happening during bar ..
            #
            # if the trap happened during bar, the relevant call line is
            # "[fuzz-exec] calling bar".
            call_start = before.rfind(FUZZ_EXEC_CALL_PREFIX, 0, trap_index)
            if call_start < 0:
                # the trap happened before we called an export, so it occured
                # during startup (the start function, or memory segment
                # operations, etc.). in that case there is nothing for us to
                # compare here; just leave.
                return
            # include the line separator in the index, as function names may
            # be prefixes of each other
            call_end = before.index(os.linesep, call_start) + 1
            # we now know the contents of the call line after which the trap
            # happens, which is something like "[fuzz-exec] calling bar", and
            # it is unique since it contains the function being called.
            call_line = before[call_start:call_end]
            trapping_export = get_export_from_call_line(call_line)

            # now that we know the trapping export, we can leave only the safe
            # ones that are before it
            safe_exports = []
            for line in before.splitlines():
                if FUZZ_EXEC_CALL_PREFIX in line:
                    export = get_export_from_call_line(line)
                    if export == trapping_export:
                        break
                    safe_exports.append(export)

            # filter out the other exports
            filtered = before_wasm + '.filtered.wasm'
            filter_exports(before_wasm, filtered, safe_exports)
            before_wasm = filtered

            # re-execute the now safe wasm
            before = run_bynterp(before_wasm, ['--fuzz-exec-before'])
            assert TRAP_PREFIX not in before, 'we should have fixed this problem'

        after_wasm_tnh = after_wasm + '.tnh.wasm'
        run([in_bin('wasm-opt'), before_wasm, '-o', after_wasm_tnh, '-tnh'] + opts + FEATURE_OPTS)
        after = run_bynterp(after_wasm_tnh, ['--fuzz-exec-before'])

        # some results cannot be compared, so we must filter them out here.
        def ignore_references(out):
            ret = []
            for line in out.splitlines():
                # only result lines are relevant here, which look like
                # [fuzz-exec] note result: foo => [...]
                if FUZZ_EXEC_NOTE_RESULT in line:
                    # we want to filter out things like "anyref(null)" or
                    # "[ref null data]".
                    if 'ref(' in line or 'ref ' in line:
                        line = line[:line.index('=>') + 2] + ' ?'
                ret.append(line)
            return '\n'.join(ret)

        before = fix_output(ignore_references(before))
        after = fix_output(ignore_references(after))

        compare_between_vms(before, after, 'TrapsNeverHappen')

    def can_run_on_wasm(self, wasm):
        # If the wasm is sensitive to changes in exports then we cannot alter
        # them, but we must remove trapping exports (see above), so we cannot
        # run in such a case.
        return not wasm_notices_export_changes(wasm)


# Tests wasm-ctor-eval
class CtorEval(TestCaseHandler):
    frequency = 0.2

    def handle(self, wasm):
        # get the expected execution results.
        wasm_exec = run_bynterp(wasm, ['--fuzz-exec-before'])

        # get the list of func exports, so we can tell ctor-eval what to eval.
        ctors = ','.join(get_exports(wasm, ['func']))
        if not ctors:
            return

        # eval the wasm.
        # we can use --ignore-external-input because the fuzzer passes in 0 to
        # all params, which is the same as ctor-eval assumes in this mode.
        evalled_wasm = wasm + '.evalled.wasm'
        output = run([in_bin('wasm-ctor-eval'), wasm, '-o', evalled_wasm, '--ctors=' + ctors, '--kept-exports=' + ctors, '--ignore-external-input'] + FEATURE_OPTS)

        # stop here if we could not eval anything at all in the module.
        if '...stopping since could not flatten memory' in output or \
           '...stopping since could not create module instance' in output:
            return
        if '...success' not in output and \
           '...partial evalling success' not in output:
            return
        evalled_wasm_exec = run_bynterp(evalled_wasm, ['--fuzz-exec-before'])

        compare_between_vms(fix_output(wasm_exec), fix_output(evalled_wasm_exec), 'CtorEval')

    def can_run_on_wasm(self, wasm):
        # ctor-eval modifies exports, because it assumes they are ctors and so
        # are only called once (so if it evals them away, they can be
        # removed). If the wasm might notice that, we cannot run.
        return not wasm_notices_export_changes(wasm)


# Tests wasm-merge
class Merge(TestCaseHandler):
    frequency = 0.15

    def handle(self, wasm):
        # generate a second wasm file to merge. note that we intentionally pick
        # a smaller size than the main wasm file, so that reduction is
        # effective (i.e., as we reduce the main wasm to small sizes, we also
        # end up with small secondary wasms)
        # TODO: add imports and exports that connect between the two
        wasm_size = os.stat(wasm).st_size
        second_size = min(wasm_size, random_size())
        second_input = abspath('second_input.dat')
        make_random_input(second_size, second_input)
        second_wasm = abspath('second.wasm')
        run([in_bin('wasm-opt'), second_input, '-ttf', '-o', second_wasm] + GEN_ARGS + FEATURE_OPTS)

        # the second wasm file must not have an export that can influence our
        # execution. the JS exports have that behavior, as when "table-set" is
        # called it will look for the export "table" on which to operate, then
        # imagine we lack that export in the first module but add it in the
        # second, then code that failed before will now use the exported table
        # from the second module (and maybe work). to avoid that, remove the
        # table export, if it exists (and if the first module doesn't export
        # it).
        second_exports = get_exports(second_wasm, ['func', 'table'])
        wasm_exports = get_exports(wasm, ['table'])
        if 'table' in second_exports and 'table' not in wasm_exports:
            filtered = [e for e in second_exports if e != 'table']
            # note we override the set of default things to keep, as we want to
            # remove the table export. doing so might change the behavior of
            # second.wasm, but that is ok.
            filter_exports(second_wasm, second_wasm, filtered, keep_defaults=False)

        # sometimes also optimize the second module
        if random.random() < 0.5:
            opts = get_random_opts()
            run([in_bin('wasm-opt'), second_wasm, '-o', second_wasm, '-all'] + FEATURE_OPTS + opts)

        # merge the wasm files. note that we must pass -all, as even if the two
        # inputs are MVP, the output may have multiple tables and multiple
        # memories (and we must also do that in the commands later down).
        #
        # Use --skip-export-conflicts as we only look at the first module's
        # exports for now - we don't care about the second module's.
        # TODO: compare the second module's exports as well, but we'd need
        #       to handle renaming of conflicting exports.
        merged = abspath('merged.wasm')
        run([in_bin('wasm-merge'), wasm, 'first',
            abspath('second.wasm'), 'second', '-o', merged,
            '--skip-export-conflicts'] + FEATURE_OPTS + ['-all'])

        # sometimes also optimize the merged module
        if random.random() < 0.5:
            opts = get_random_opts()
            run([in_bin('wasm-opt'), merged, '-o', merged, '-all'] + FEATURE_OPTS + opts)

        # verify that merging in the second module did not alter the output.
        output = run_bynterp(wasm, ['--fuzz-exec-before', '-all'])
        output = fix_output(output)
        merged_output = run_bynterp(merged, ['--fuzz-exec-before', '-all'])
        merged_output = fix_output(merged_output)

        # a complication is that the second module's exports are appended, so we
        # have extra output. to handle that, just prune the tail, so that we
        # only compare the original exports from the first module.
        # TODO: compare the second module's exports to themselves as well, but
        #       they may have been renamed due to overlaps...
        merged_output = merged_output[:len(output)]

        compare_between_vms(output, merged_output, 'Merge')

    def can_run_on_wasm(self, wasm):
        # wasm-merge combines exports, which can alter their indexes and lead to
        # noticeable differences if the wasm is sensitive to such things, which
        # prevents us from running.
        return not wasm_notices_export_changes(wasm)


FUNC_NAMES_REGEX = re.compile(r'\n [(]func [$](\S+)')


# Tests wasm-split
class Split(TestCaseHandler):
    frequency = 1  # TODO: adjust lower when we actually enable this

    def handle(self, wasm):
        # get the list of function names, some of which we will decide to split
        # out
        wat = run([in_bin('wasm-dis'), wasm] + FEATURE_OPTS)
        all_funcs = re.findall(FUNC_NAMES_REGEX, wat)

        # get the original output before splitting
        output = run_d8_wasm(wasm)
        output = fix_output(output)

        # find the names of the exports. we need this because when we split the
        # module then new exports appear to connect the two halves of the
        # original module. we do not want to call all the exports on the new
        # primary module, but only the original ones.
        exports = []
        for line in output.splitlines():
            if FUZZ_EXEC_CALL_PREFIX in line:
                exports.append(get_export_from_call_line(line))

        # pick which to split out, with a random rate of picking (biased towards
        # 0.5).
        rate = (random.random() + random.random()) / 2
        split_funcs = []
        for func in all_funcs:
            if random.random() < rate:
                split_funcs.append(func)

        if not split_funcs:
            # nothing to split out
            return

        # split the wasm into two
        primary = wasm + '.primary.wasm'
        secondary = wasm + '.secondary.wasm'

        # we require reference types, because that allows us to create our own
        # table. without that we use the existing table, and that may interact
        # with user code in odd ways (it really only works with the particular
        # form of table+segments that LLVM emits, and not with random fuzzer
        # content).
        split_feature_opts = FEATURE_OPTS + ['--enable-reference-types']

        run([in_bin('wasm-split'), wasm, '--split',
             '--split-funcs', ','.join(split_funcs),
             '--primary-output', primary,
             '--secondary-output', secondary] + split_feature_opts)

        # sometimes also optimize the split modules
        optimized = False

        def optimize(name):
            # do not optimize if it would change the ABI
            if CLOSED_WORLD:
                return name
            # TODO: use other optimizations here, but we'd need to be careful of
            #       anything that can alter the ABI, and also current
            #       limitations of open-world optimizations (see discussion in
            #       https://github.com/WebAssembly/binaryen/pull/6660)
            opts = ['-O3']
            new_name = name + '.opt.wasm'
            run([in_bin('wasm-opt'), name, '-o', new_name, '-all'] + opts + split_feature_opts)
            nonlocal optimized
            optimized = True
            return new_name

        if random.random() < 0.5:
            primary = optimize(primary)
        if random.random() < 0.5:
            secondary = optimize(secondary)

        # prepare the list of exports to call. the format is
        #
        #  exports:A,B,C
        #
        exports_to_call = 'exports:' + ','.join(exports)

        # get the output from the split modules, linking them using JS
        # TODO run liftoff/turboshaft/etc.
        linked_output = run_d8_wasm(primary, args=[secondary, exports_to_call])
        linked_output = fix_output(linked_output)

        # see D8.can_compare_to_self: we cannot compare optimized outputs if
        # NaNs are allowed, as the optimizer can modify NaNs differently than
        # the JS engine.
        if not (NANS and optimized):
            compare_between_vms(output, linked_output, 'Split')

    def can_run_on_wasm(self, wasm):
        # to run the split wasm we use JS, that is, JS links the exports of one
        # to the imports of the other, etc. since we run in JS, the wasm must be
        # valid for JS.
        if not LEGALIZE:
            return False

        # see D8.can_run
        return all_disallowed(['shared-everything'])


# Check that the text format round-trips without error.
class RoundtripText(TestCaseHandler):
    frequency = 0.05

    def handle(self, wasm):
        # use name-types because in wasm GC we can end up truncating the default
        # names which are very long, causing names to collide and the wast to be
        # invalid
        # FIXME: run name-types by default during load?
        run([in_bin('wasm-opt'), wasm, '--name-types', '-S', '-o', abspath('a.wast')] + FEATURE_OPTS)
        run([in_bin('wasm-opt'), abspath('a.wast')] + FEATURE_OPTS)


# Fuzz in a near-identical manner to how we fuzz on ClusterFuzz. This is mainly
# to see that fuzzing that way works properly (it likely won't catch anything
# the other fuzzers here catch, though it is possible). That is, running this
# script continuously will give continuous cover that ClusterFuzz should be
# running ok.
#
# Note that this is *not* deterministic like the other fuzzers: it runs run.py
# like ClusterFuzz does, and that generates its own random data. If a bug is
# caught here, it must be reduced manually.
class ClusterFuzz(TestCaseHandler):
    frequency = 0.1

    def handle(self, wasm):
        self.ensure()

        # run.py() should emit these two files. Delete them to make sure they
        # are created by run.py() in the next step.
        fuzz_file = 'fuzz-binaryen-1.js'
        flags_file = 'flags-binaryen-1.js'
        for f in [fuzz_file, flags_file]:
            if os.path.exists(f):
                os.unlink(f)

        # Call run.py(), similarly to how ClusterFuzz does.
        run([sys.executable,
             os.path.join(self.clusterfuzz_dir, 'run.py'),
             '--output_dir=' + os.getcwd(),
             '--no_of_files=1'])

        # We should see the two files.
        assert os.path.exists(fuzz_file)
        assert os.path.exists(flags_file)

        # Run the testcase in V8, similarly to how ClusterFuzz does.
        cmd = [shared.V8]
        # The flags are given in the flags file - we do *not* use our normal
        # flags here!
        with open(flags_file, 'r') as f:
            flags = f.read()
        cmd.append(flags)
        # Run the fuzz file, which contains a modified fuzz_shell.js - we do
        # *not* run fuzz_shell.js normally.
        cmd.append(os.path.abspath(fuzz_file))
        # No wasm file needs to be provided: it is hardcoded into the JS. Note
        # that we use run_vm(), which will ignore known issues in our output and
        # in V8. Those issues may cause V8 to e.g. reject a binary we emit that
        # is invalid, but that should not be a problem for ClusterFuzz (it isn't
        # a crash).
        output = run_vm(cmd)

        # Verify that we called something. The fuzzer should always emit at
        # least one exported function (unless we've decided to ignore the entire
        # run).
        if output != IGNORE:
            assert FUZZ_EXEC_CALL_PREFIX in output

    def ensure(self):
        # The first time we actually run, set things up: make a bundle like the
        # one ClusterFuzz receives, and unpack it for execution into a dir. The
        # existence of that dir shows we've ensured all we need.
        if hasattr(self, 'clusterfuzz_dir'):
            return

        self.clusterfuzz_dir = 'clusterfuzz'
        if os.path.exists(self.clusterfuzz_dir):
            shutil.rmtree(self.clusterfuzz_dir)
        os.mkdir(self.clusterfuzz_dir)

        print('Bundling for ClusterFuzz')
        bundle = 'fuzz_opt_clusterfuzz_bundle.tgz'
        run([in_binaryen('scripts', 'bundle_clusterfuzz.py'), bundle])

        print('Unpacking for ClusterFuzz')
        tar = tarfile.open(bundle, "r:gz")
        tar.extractall(path=self.clusterfuzz_dir)
        tar.close()


# The global list of all test case handlers
testcase_handlers = [
    FuzzExec(),
    CompareVMs(),
    CheckDeterminism(),
    Wasm2JS(),
    TrapsNeverHappen(),
    CtorEval(),
    Merge(),
    # TODO: enable when stable enough, and adjust |frequency| (see above)
    # Split(),
    RoundtripText(),
    ClusterFuzz(),
]


test_suffixes = ['*.wasm', '*.wast', '*.wat']

core_tests = shared.get_tests(shared.get_test_dir('.'), test_suffixes)
passes_tests = shared.get_tests(shared.get_test_dir('passes'), test_suffixes)
spec_tests = shared.get_tests(shared.get_test_dir('spec'), test_suffixes)
wasm2js_tests = shared.get_tests(shared.get_test_dir('wasm2js'), test_suffixes)
lld_tests = shared.get_tests(shared.get_test_dir('lld'), test_suffixes)
unit_tests = shared.get_tests(shared.get_test_dir(os.path.join('unit', 'input')), test_suffixes)
lit_tests = shared.get_tests(shared.get_test_dir('lit'), test_suffixes, recursive=True)
all_tests = core_tests + passes_tests + spec_tests + wasm2js_tests + lld_tests + unit_tests + lit_tests


# Do one test, given an input file for -ttf and some optimizations to run
def test_one(random_input, given_wasm):
    randomize_pass_debug()
    randomize_feature_opts()
    randomize_fuzz_settings()
    pick_initial_contents()

    opts = get_random_opts()
    print('randomized opts:', '\n  ' + '\n  '.join(opts))
    print()

    if given_wasm:
        # if given a wasm file we want to use it as is, but we also want to
        # apply properties like not having any NaNs, which the original fuzz
        # wasm had applied. that is, we need to preserve properties like not
        # having nans through reduction.
        try:
            run([in_bin('wasm-opt'), given_wasm, '-o', abspath('a.wasm')] + GEN_ARGS + FEATURE_OPTS)
        except Exception as e:
            print("Internal error in fuzzer! Could not run given wasm")
            raise e
    else:
        # emit the target features section so that reduction can work later,
        # without needing to specify the features
        generate_command = [in_bin('wasm-opt'), random_input, '-ttf', '-o', abspath('a.wasm')] + GEN_ARGS + FEATURE_OPTS
        if INITIAL_CONTENTS:
            generate_command += ['--initial-fuzz=' + INITIAL_CONTENTS]
        if PRINT_WATS:
            printed = run(generate_command + ['--print'])
            with open('a.printed.wast', 'w') as f:
                f.write(printed)
        else:
            run(generate_command)
    wasm_size = os.stat('a.wasm').st_size
    bytes = wasm_size
    print('pre wasm size:', wasm_size)
    update_feature_opts('a.wasm')

    # create a second (optimized) wasm for handlers that want to look at pairs.
    generate_command = [in_bin('wasm-opt'), abspath('a.wasm'), '-o', abspath('b.wasm')] + opts + FUZZ_OPTS + FEATURE_OPTS
    if PRINT_WATS:
        printed = run(generate_command + ['--print'])
        with open('b.printed.wast', 'w') as f:
            f.write(printed)
    else:
        run(generate_command)
    wasm_size = os.stat('b.wasm').st_size
    bytes += wasm_size
    print('post wasm size:', wasm_size)

    # First, find which handlers can even run here. Note that we check a.wasm
    # and not b.wasm (optimizations do not change fuzzability).
    relevant_handlers = [handler for handler in testcase_handlers if not hasattr(handler, 'get_commands') and handler.can_run_on_wasm('a.wasm')]
    if len(relevant_handlers) == 0:
        return 0
    # filter by frequency
    filtered_handlers = [handler for handler in relevant_handlers if random.random() < handler.frequency]
    if len(filtered_handlers) == 0:
        # pick at least one, to not waste the effort we put into making the wasm
        filtered_handlers = [random.choice(relevant_handlers)]
    # run only some of the pair handling handlers. if we ran them all all the
    # time that would mean we have less variety in wasm files and passes run
    # on them in the same amount of time.
    NUM_PAIR_HANDLERS = 3
    used_handlers = set()
    for i in range(NUM_PAIR_HANDLERS):
        testcase_handler = random.choice(filtered_handlers)
        if testcase_handler in used_handlers:
            continue
        used_handlers.add(testcase_handler)
        assert testcase_handler.can_run_on_wasm('a.wasm')
        print('running testcase handler:', testcase_handler.__class__.__name__)
        testcase_handler.increment_runs()

        # let the testcase handler handle this testcase however it wants. in this case we give it
        # the input and both wasms.
        testcase_handler.handle_pair(input=random_input, before_wasm=abspath('a.wasm'), after_wasm=abspath('b.wasm'), opts=opts + FEATURE_OPTS)
        print('')

    return bytes


def write_commands(commands, filename):
    with open(filename, 'w') as f:
        f.write('set -e\n')
        for command in commands:
            f.write('echo "%s"\n' % command)
            pre = 'BINARYEN_PASS_DEBUG=%s ' % (os.environ.get('BINARYEN_PASS_DEBUG') or '0')
            f.write(pre + command + ' &> /dev/null\n')
        f.write('echo "ok"\n')


# main

opt_choices = [
    (),
    ('-O1',), ('-O2',), ('-O3',), ('-O4',), ('-Os',), ('-Oz',),
    ("--abstract-type-refining",),
    ("--cfp",),
    ("--coalesce-locals",),
    # XXX slow, non-default ("--coalesce-locals-learning",),
    ("--code-pushing",),
    ("--code-folding",),
    ("--const-hoisting",),
    ("--dae",),
    ("--dae-optimizing",),
    ("--dce",),
    ("--directize",),
    ("--discard-global-effects",),
    ("--flatten", "--dfo",),
    ("--duplicate-function-elimination",),
    ("--flatten",),
    # ("--fpcast-emu",), # removes indirect call failures as it makes them go through regardless of type
    ("--inlining",),
    ("--inlining-optimizing",),
    ("--flatten", "--simplify-locals-notee-nostructure", "--local-cse",),
    # note that no pass we run here should add effects to a function, so it is
    # ok to run this pass and let the passes after it use the effects to
    # optimize
    ("--generate-global-effects",),
    ("--global-refining",),
    ("--gsi",),
    ("--gto",),
    ("--gufa",),
    ("--gufa-cast-all",),
    ("--gufa-optimizing",),
    ("--local-cse",),
    ("--heap2local",),
    ("--remove-unused-names", "--heap2local",),
    ("--heap-store-optimization",),
    ("--generate-stack-ir",),
    ("--licm",),
    ("--local-subtyping",),
    ("--memory-packing",),
    ("--merge-blocks",),
    ('--merge-locals',),
    # test a few monomorphization levels, and also -always
    ('--monomorphize', '--pass-arg=monomorphize-min-benefit@0'),
    ('--monomorphize', '--pass-arg=monomorphize-min-benefit@50'),
    ('--monomorphize', '--pass-arg=monomorphize-min-benefit@95'),
    ('--monomorphize-always',),
    ('--minimize-rec-groups',),
    ('--no-stack-ir',),
    ('--once-reduction',),
    ("--optimize-casts",),
    ("--optimize-instructions",),
    ("--optimize-stack-ir",),
    ("--generate-stack-ir", "--optimize-stack-ir",),
    # the full lifecycle of stack IR: generate, optimize, and write (and read)
    ("--generate-stack-ir", "--optimize-stack-ir", "--roundtrip"),
    ("--pick-load-signs",),
    ("--precompute",),
    ("--precompute-propagate",),
    ("--print",),
    ("--remove-unused-brs",),
    ("--remove-unused-nonfunction-module-elements",),
    ("--remove-unused-module-elements",),
    ("--remove-unused-names",),
    ("--remove-unused-types",),
    ("--reorder-functions",),
    ("--reorder-locals",),
    ("--flatten", "--rereloop",),
    ("--roundtrip",),
    ("--rse",),
    ("--signature-pruning",),
    ("--signature-refining",),
    ("--simplify-globals",),
    ("--simplify-globals-optimizing",),
    ("--simplify-locals",),
    ("--simplify-locals-nonesting",),
    ("--simplify-locals-nostructure",),
    ("--simplify-locals-notee",),
    ("--simplify-locals-notee-nostructure",),
    ("--ssa",),
    ("--tuple-optimization",),
    ("--type-finalizing",),
    ("--type-refining",),
    ("--type-merging",),
    ("--type-ssa",),
    ("--type-unfinalizing",),
    ("--unsubtyping",),
    ("--vacuum",),
]

# TODO: Fix these passes so that they still work without --closed-world!
requires_closed_world = {("--type-refining",),
                         ("--signature-pruning",),
                         ("--signature-refining",),
                         ("--gto",),
                         ("--remove-unused-types",),
                         ("--abstract-type-refining",),
                         ("--cfp",),
                         ("--gsi",),
                         ("--type-ssa",),
                         ("--type-merging",)}


def get_random_opts():
    flag_groups = []
    has_flatten = False

    if CLOSED_WORLD:
        usable_opt_choices = opt_choices
    else:
        usable_opt_choices = [choice
                              for choice in opt_choices
                              if choice not in requires_closed_world]

    # core opts
    while 1:
        choice = random.choice(usable_opt_choices)
        if '--flatten' in choice or '-O4' in choice:
            if has_flatten:
                print('avoiding multiple --flatten in a single command, due to exponential overhead')
                continue
            if '--enable-multivalue' in FEATURE_OPTS and '--enable-reference-types' in FEATURE_OPTS:
                print('avoiding --flatten due to multivalue + reference types not supporting it (spilling of non-nullable tuples)')
                print('TODO: Resolving https://github.com/WebAssembly/binaryen/issues/4824 may fix this')
                continue
            if '--enable-exception-handling' in FEATURE_OPTS:
                print('avoiding --flatten due to exception-handling not supporting it (requires blocks with results)')
                continue
            if '--gc' not in FEATURE_OPTS:
                print('avoiding --flatten due to GC not supporting it (spilling of non-nullable locals)')
                continue
            if INITIAL_CONTENTS and os.path.getsize(INITIAL_CONTENTS) > 2000:
                print('avoiding --flatten due using a large amount of initial contents, which may blow up')
                continue
            else:
                has_flatten = True
        if ('--rereloop' in choice or '--dfo' in choice) and \
           '--enable-exception-handling' in FEATURE_OPTS:
            print('avoiding --rereloop or --dfo due to exception-handling not supporting it')
            continue
        flag_groups.append(choice)
        if len(flag_groups) > 20 or random.random() < 0.3:
            break
    # maybe add an extra round trip
    if random.random() < 0.5:
        pos = random.randint(0, len(flag_groups))
        flag_groups = flag_groups[:pos] + [('--roundtrip',)] + flag_groups[pos:]
    ret = [flag for group in flag_groups for flag in group]
    # modifiers (if not already implied by a -O? option)
    if '-O' not in str(ret):
        if random.random() < 0.5:
            ret += ['--optimize-level=' + str(random.randint(0, 3))]
        if random.random() < 0.5:
            ret += ['--shrink-level=' + str(random.randint(0, 3))]
    # possibly converge. don't do this very often as it can be slow.
    if random.random() < 0.05:
        ret += ['--converge']
    # possibly inline all the things as much as possible. inlining that much may
    # be realistic in some cases (on GC benchmarks it is very helpful), but
    # also, inlining so much allows other optimizations to kick in, which
    # increases coverage
    # (the specific number here doesn't matter, but it is far higher than the
    # wasm limitation on function body size which is 128K)
    if random.random() < 0.5:
        ret += ['-fimfs=99999999']
    # the default for partial-inlining-ifs is 0, so also test with a realistic
    # value (the same used in j2wasm atm)
    if random.random() < 0.5:
        ret += ['-pii=4']
    # test both closed and open world
    if CLOSED_WORLD:
        ret += [CLOSED_WORLD_FLAG]
    assert ret.count('--flatten') <= 1
    return ret


# main

# list of all the flags to disable all the features. if all of these are added
# then we target the MVP.
FEATURE_DISABLE_FLAGS = run([in_bin('wasm-opt'), '--print-features', in_binaryen('test', 'hello_world.wat')] + CONSTANT_FEATURE_OPTS).replace('--enable', '--disable').strip().split('\n')
print('FEATURE_DISABLE_FLAGS:', FEATURE_DISABLE_FLAGS)

# some features depend on other features, so if a required feature is
# disabled, its dependent features need to be disabled as well.
IMPLIED_FEATURE_OPTS = {
    '--disable-reference-types': ['--disable-gc', '--disable-exception-handling', '--disable-strings'],
    '--disable-gc': ['--disable-strings'],
}

print('''
<<< fuzz_opt.py >>>
''')

if not shared.V8:
    print('The v8 shell, d8, must be in the path')
    sys.exit(1)

if __name__ == '__main__':
    # if we are given a seed, run exactly that one testcase. otherwise,
    # run new ones until we fail
    # if we are given a seed, we can also be given a wasm file, which we use
    # instead of the randomly generating one. this can be useful for
    # reduction.
    given_wasm = None
    if len(shared.requested) >= 1:
        given_seed = int(shared.requested[0])
        print('checking a single given seed', given_seed)
        if len(shared.requested) >= 2:
            given_wasm = shared.requested[1]
            print('using given wasm file', given_wasm)
    else:
        given_seed = None
        print('checking infinite random inputs')

    init_important_initial_contents()

    seed = time.time() * os.getpid()
    raw_input_data = abspath('input.dat')
    counter = 0
    total_wasm_size = 0
    total_input_size = 0
    total_input_size_squares = 0
    start_time = time.time()
    while True:
        counter += 1
        if given_seed is not None:
            seed = given_seed
            given_seed_passed = True
        else:
            seed = random.randint(0, 1 << 64)
        random.seed(seed)
        input_size = random_size()
        total_input_size += input_size
        total_input_size_squares += input_size ** 2
        print('')
        mean = float(total_input_size) / counter
        mean_of_squares = float(total_input_size_squares) / counter
        stddev = math.sqrt(mean_of_squares - (mean ** 2))
        elapsed = max(0.000001, time.time() - start_time)
        print('ITERATION:', counter, 'seed:', seed, 'size:', input_size,
              '(mean:', str(mean) + ', stddev:', str(stddev) + ')',
              'speed:', counter / elapsed, 'iters/sec, ',
              total_wasm_size / counter, 'wasm_bytes/iter')
        if ignored_vm_runs:
            print(f'(ignored {ignored_vm_runs} iters, for reasons {ignored_vm_run_reasons})')
        print()
        make_random_input(input_size, raw_input_data)
        assert os.path.getsize(raw_input_data) == input_size
        # remove the generated wasm file, so that we can tell if the fuzzer
        # fails to create one
        if os.path.exists('a.wasm'):
            os.remove('a.wasm')
        # run an iteration of the fuzzer
        try:
            total_wasm_size += test_one(raw_input_data, given_wasm)
        except KeyboardInterrupt:
            print('(stopping by user request)')
            break
        except Exception as e:
            # print the exception manually, so that we can show our message at
            # the very end where it won't be missed
            ex_type, ex, tb = sys.exc_info()
            print('!')
            print('-----------------------------------------')
            print('Exception:')
            traceback.print_tb(tb)
            print('-----------------------------------------')
            print('!')
            for arg in e.args:
                print(arg)
            if given_seed is not None:
                given_seed_passed = False

            # We want to generate a template reducer script only when there is
            # no given wasm file. That we have a given wasm file means we are no
            # longer working on the original test case but modified one, which
            # is likely to be called within wasm-reduce script itself, so
            # original.wasm and reduce.sh should not be overwritten.
            if not given_wasm:
                # We can't do this if a.wasm doesn't exist, which can be the
                # case if we failed to even generate the wasm.
                if not os.path.exists('a.wasm'):
                    print('''\
================================================================================
You found a bug in the fuzzer itself! It failed to generate a valid wasm file
from the random input. Please report it with

  seed: %(seed)d

and the exact version of Binaryen you found it on, plus the exact Python
version (hopefully deterministic random numbers will be identical).

You can run that testcase again with "fuzz_opt.py %(seed)d"

(We can't automatically reduce this testcase since we can only run the reducer
on valid wasm files.)
================================================================================
                ''' % {'seed': seed})
                    break
                # show some useful info about filing a bug and reducing the
                # testcase (to make reduction simple, save "original.wasm" on
                # the side, so that we can autoreduce using the name "a.wasm"
                # which we use internally)
                original_wasm = abspath('original.wasm')
                shutil.copyfile('a.wasm', original_wasm)
                # write out a useful reduce.sh
                auto_init = ''
                if not shared.options.auto_initial_contents:
                    auto_init = '--no-auto-initial-contents'
                with open('reduce.sh', 'w') as reduce_sh:
                    reduce_sh.write('''\
# check the input is even a valid wasm file
echo "The following value should be 0:"
%(wasm_opt)s %(features)s %(temp_wasm)s
echo "  " $?

# run the command
echo "The following value should be 1:"
./scripts/fuzz_opt.py %(auto_init)s --binaryen-bin %(bin)s %(seed)d %(temp_wasm)s > o 2> e
echo "  " $?

#
# You may want to print out part of "o" or "e", if the output matters and not
# just the return code. For example,
#
#   cat o | tail -n 10
#
# would print out the last few lines of stdout, which might be useful if that
# mentions the specific error you want. Make sure that includes the right
# details (sometimes stderr matters too), and preferably no more (less details
# allow more reduction, but raise the risk of it reducing to something you don't
# quite want).
#
# To do a "dry run" of what the reducer will do, copy the original file to the
# test file that this script will run on,
#
#   cp %(original_wasm)s %(temp_wasm)s
#
# and then run
#
#   bash %(reduce_sh)s
#
# You may also need to add  --timeout 5  or such if the testcase is a slow one.
#
                  ''' % {'wasm_opt': in_bin('wasm-opt'),
                         'bin': shared.options.binaryen_bin,
                         'seed': seed,
                         'auto_init': auto_init,
                         'original_wasm': original_wasm,
                         'temp_wasm': abspath('t.wasm'),
                         'features': ' '.join(FEATURE_OPTS),
                         'reduce_sh': abspath('reduce.sh')})

                print('''\
================================================================================
You found a bug! Please report it with

  seed: %(seed)d

and the exact version of Binaryen you found it on, plus the exact Python
version (hopefully deterministic random numbers will be identical).

You can run that testcase again with "fuzz_opt.py %(seed)d"

The initial wasm file used here is saved as %(original_wasm)s

You can reduce the testcase by running this now:

||||
vvvv


%(wasm_reduce)s %(features)s %(original_wasm)s '--command=bash %(reduce_sh)s' -t %(temp_wasm)s -w %(working_wasm)s


^^^^
||||

Make sure to verify by eye that the output says something like this:

The following value should be 0:
  0
The following value should be 1:
  1

(If it does not, then one possible issue is that the fuzzer fails to write a
valid binary. If so, you can print the output of the fuzzer's first command
(using -ttf / --translate-to-fuzz) in text form and run the reduction from that,
passing --text to the reducer.)

You can also read "%(reduce_sh)s" which has been filled out for you and includes
docs and suggestions.

After reduction, the reduced file will be in %(working_wasm)s
================================================================================
                ''' % {'seed': seed,
                       'original_wasm': original_wasm,
                       'temp_wasm': abspath('t.wasm'),
                       'working_wasm': abspath('w.wasm'),
                       'wasm_reduce': in_bin('wasm-reduce'),
                       'reduce_sh': abspath('reduce.sh'),
                       'features': ' '.join(FEATURE_OPTS)})
                break
        if given_seed is not None:
            break

        print('\nInvocations so far:')
        for testcase_handler in testcase_handlers:
            print('  ', testcase_handler.__class__.__name__ + ':', testcase_handler.count_runs())

    if given_seed is not None:
        if given_seed_passed:
            print('(finished running seed %d without error)' % given_seed)
            sys.exit(0)
        else:
            print('(finished running seed %d, see error above)' % given_seed)
            sys.exit(1)
