import os

from scripts.test import shared
from . import utils


class FeatureValidationTest(utils.BinaryenTestCase):
    def check_feature(self, module, error, flag, const_flags=[]):
        p = shared.run_process(shared.WASM_OPT +
                               ['--mvp-features', '--print', '-o', os.devnull] +
                               const_flags,
                               input=module, check=False, capture_output=True)
        self.assertIn(error, p.stderr)
        self.assertIn('Fatal: error validating input', p.stderr)
        self.assertNotEqual(p.returncode, 0)
        p = shared.run_process(
            shared.WASM_OPT + ['--mvp-features', '--print', '-o', os.devnull] +
            const_flags + [flag],
            input=module,
            check=False,
            capture_output=True)
        self.assertEqual(p.returncode, 0)

    def check_simd(self, module, error):
        self.check_feature(module, error, '--enable-simd')

    def check_sign_ext(self, module, error):
        self.check_feature(module, error, '--enable-sign-ext')

    def check_bulk_mem(self, module, error):
        self.check_feature(module, error, '--enable-bulk-memory')

    def check_bulk_mem_opt(self, module, error):
        self.check_feature(module, error, '--enable-bulk-memory-opt')

    def check_exception_handling(self, module, error):
        self.check_feature(module, error, '--enable-exception-handling')

    def check_tail_call(self, module, error):
        self.check_feature(module, error, '--enable-tail-call')

    def check_reference_types(self, module, error):
        self.check_feature(module, error, '--enable-reference-types')

    def check_multivalue(self, module, error):
        self.check_feature(module, error, '--enable-multivalue')

    def check_multivalue_exception_handling(self, module, error):
        self.check_feature(module, error, '--enable-multivalue',
                           ['--enable-exception-handling'])

    def check_gc(self, module, error):
        # GC implies reference types
        self.check_feature(module, error, '--enable-gc',
                           ['--enable-reference-types'])

    def check_stack_switching(self, module, error):
        # Stack switching implies function references (which is provided by
        # gc in binaryen, and implies reference types) and exceptions
        self.check_feature(module, error, '--enable-stack-switching',
                           ['--enable-gc', '--enable-reference-types', '--enable-exception-handling'])

    def test_v128_signature(self):
        module = '''
        (module
         (func $foo (param $0 v128) (result v128)
            (local.get $0)
         )
        )
        '''
        self.check_simd(module, 'all used types should be allowed')

    def test_v128_global(self):
        module = '''
        (module
         (global $foo (mut v128) (v128.const i32x4 0 0 0 0))
        )
        '''
        self.check_simd(module, 'all used types should be allowed')

    def test_v128_local(self):
        module = '''
        (module
         (func $foo
            (local v128)
         )
        )
        '''
        self.check_simd(module, 'all used types should be allowed')

    def test_simd_const(self):
        module = '''
        (module
         (func $foo
            (drop (v128.const i32x4 0 0 0 0))
         )
        )
        '''
        self.check_simd(module, 'all used features should be allowed')

    def test_simd_load(self):
        module = '''
        (module
         (memory 1 1)
         (func $foo
            (drop (v128.load (i32.const 0)))
         )
        )
        '''
        self.check_simd(module, 'SIMD operations require SIMD [--enable-simd]')

    def test_simd_splat(self):
        module = '''
        (module
         (func $foo
            (drop (i32x4.splat (i32.const 0)))
         )
        )
        '''
        self.check_simd(module, 'all used features should be allowed')

    def test_sign_ext(self):
        module = '''
        (module
         (func $foo
            (drop (i32.extend8_s (i32.const 7)))
         )
        )
        '''
        self.check_sign_ext(module, 'all used features should be allowed')

    def test_bulk_mem_inst(self):
        module = '''
        (module
         (memory 1 1)
         (func $foo
            (memory.copy (i32.const 0) (i32.const 8) (i32.const 8))
         )
        )
        '''
        self.check_bulk_mem_opt(module,
                                'memory.copy operations require bulk memory operations [--enable-bulk-memory-opt]')
        # Test that enabling bulk-memory also enables bulk-memory-opt
        self.check_bulk_mem(module,
                            'memory.copy operations require bulk memory operations [--enable-bulk-memory-opt]')

    def test_bulk_mem_segment(self):
        module = '''
        (module
         (memory 256 256)
         (data "42")
        )
        '''
        self.check_bulk_mem(module, 'nonzero segment flags require bulk memory [--enable-bulk-memory]')

    def test_tail_call(self):
        module = '''
        (module
         (func $bar)
         (func $foo
            (return_call $bar)
         )
        )
        '''
        self.check_tail_call(module, 'return_call* requires tail calls [--enable-tail-call]')

    def test_tail_call_indirect(self):
        module = '''
        (module
         (type $T (func))
         (table $0 1 1 funcref)
         (func $foo
            (return_call_indirect (type $T)
             (i32.const 0)
            )
         )
        )
        '''
        self.check_tail_call(module, 'return_call* requires tail calls [--enable-tail-call]')

    def test_reference_types_externref(self):
        module = '''
        (module
         (import "env" "test1" (func $test1 (param externref) (result externref)))
         (import "env" "test2" (global $test2 externref))
         (export "test1" (func $test1))
         (export "test2" (global $test2))
         (func $externref_test (param $0 externref) (result externref)
          (return
           (call $test1
            (local.get $0)
           )
          )
         )
        )
        '''
        self.check_reference_types(module, 'all used types should be allowed')

    def test_tag(self):
        module = '''
        (module
         (tag $e (param i32))
         (func $foo
            (throw $e (i32.const 0))
         )
        )
        '''
        self.check_exception_handling(module, 'Tags require exception-handling [--enable-exception-handling]')

    def test_multivalue_import(self):
        module = '''
        (module
         (import "env" "foo" (func $foo (result i32 i64)))
        )
        '''
        self.check_multivalue(module, 'Imported multivalue function requires multivalue [--enable-multivalue]')

    def test_multivalue_function(self):
        module = '''
        (module
         (func $foo (result i32 i64)
          (tuple.make 2
           (i32.const 42)
           (i64.const 42)
          )
         )
        )
        '''
        self.check_multivalue(module, 'Multivalue function results ' +
                              '(multivalue is not enabled)')

    def test_multivalue_tag(self):
        module = '''
        (module
         (tag $foo (param i32 i64))
        )
        '''
        self.check_multivalue_exception_handling(module, 'Multivalue tag type requires multivalue [--enable-multivalue]')

    def test_multivalue_block(self):
        module = '''
        (module
         (func $foo
          (tuple.drop 2
           (block (result i32 i64)
            (tuple.make 2
             (i32.const 42)
             (i64.const 42)
            )
           )
          )
         )
        )
        '''
        self.check_multivalue(module, 'Multivalue block type require multivalue [--enable-multivalue]')

    def test_i31_global(self):
        module = '''
        (module
         (global $foo (ref null i31) (ref.null i31))
        )
        '''
        self.check_gc(module, 'all used types should be allowed')

    def test_i31_local(self):
        module = '''
        (module
         (func $foo
          (local $0 (ref null i31))
         )
        )
        '''
        self.check_gc(module, 'all used types should be allowed')

    def test_eqref_global(self):
        module = '''
        (module
         (global $foo eqref (ref.null eq))
        )
        '''
        self.check_gc(module, 'all used types should be allowed')

    def test_eqref_local(self):
        module = '''
        (module
         (func $foo
          (local $0 eqref)
         )
        )
        '''
        self.check_gc(module, 'all used types should be allowed')

    def test_tag_results(self):
        module = '''
        (module
         (tag $foo (result i32))
        )
        '''
        self.check_stack_switching(module,
                                   'Tags with result types require stack '
                                   'switching feature [--enable-stack-switching]')

    def test_cont_type(self):
        module = '''
        (module
         (type $ft (func (param i32) (result i32)))
         (type $ct (cont $ft))
         (func $foo
          (local $0 (ref $ct))
         )
        )
        '''
        self.check_stack_switching(module, 'all used types should be allowed')

    def test_call_indirect_overlong(self):
        # Check that the call-indirect-overlong enable and disable are ignored.
        module = '''
        (module)
        '''

        def check_nop(flag):
            p = shared.run_process(
                shared.WASM_OPT + ['--mvp-features', '--print', '-o', os.devnull] +
                [flag],
                input=module,
                check=False,
                capture_output=True)
            self.assertEqual(p.returncode, 0)
        check_nop('--enable-call-indirect-overlong')
        check_nop('--disable-call-indirect-overlong')


class TargetFeaturesSectionTest(utils.BinaryenTestCase):
    def test_atomics(self):
        filename = 'atomics_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['threads'])
        self.assertIn('i32.atomic.rmw.add', self.disassemble(filename))

    def test_bulk_memory_opt(self):
        filename = 'bulkmem_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['bulk-memory-opt'])
        self.assertIn('memory.copy', self.disassemble(filename))

    def test_nontrapping_fptoint(self):
        filename = 'truncsat_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['nontrapping-float-to-int'])
        self.assertIn('i32.trunc_sat_f32_u', self.disassemble(filename))

    def test_mutable_globals(self):
        filename = 'mutable_globals_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['mutable-globals'])
        self.assertIn('(import "env" "global-mut" (global $gimport$0 (mut i32)))',
                      self.disassemble(filename))

    def test_sign_ext(self):
        filename = 'signext_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['sign-ext'])
        self.assertIn('i32.extend8_s', self.disassemble(filename))

    def test_simd(self):
        filename = 'simd_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['simd'])
        self.assertIn('i32x4.splat', self.disassemble(filename))

    def test_tailcall(self):
        filename = 'tail_call_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['tail-call'])
        self.assertIn('return_call', self.disassemble(filename))

    def test_reference_types(self):
        filename = 'reference_types_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['reference-types'])
        self.assertIn('anyref', self.disassemble(filename))

    def test_exception_handling(self):
        filename = 'exception_handling_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['exception-handling'])
        self.assertIn('throw', self.disassemble(filename))

    def test_gc(self):
        filename = 'gc_target_feature.wasm'
        self.roundtrip(filename)
        self.check_features(filename, ['reference-types', 'gc'])
        disassembly = self.disassemble(filename)
        self.assertIn('externref', disassembly)
        self.assertIn('eqref', disassembly)

    def test_superset(self):
        # It is ok to enable additional features past what is in the section.
        shared.run_process(
            shared.WASM_OPT + ['--print', '--detect-features', '-mvp',
                               '--enable-simd', '--enable-sign-ext',
                               self.input_path('signext_target_feature.wasm')])

    def test_superset_even_without_detect_features(self):
        # It is ok to enable additional features past what is in the section,
        # even without passing --detect-features (which is now a no-op).
        path = self.input_path('signext_target_feature.wasm')
        shared.run_process(
            shared.WASM_OPT + ['--print', '--enable-simd', '-o', os.devnull,
                               path])

    def test_superset_with_detect_features(self):
        path = self.input_path('signext_target_feature.wasm')
        shared.run_process(
            shared.WASM_OPT + ['--print', '--detect-features',
                               '--enable-simd', '-o', os.devnull, path])

    def test_explicit_detect_features(self):
        self.check_features('signext_target_feature.wasm', ['simd', 'sign-ext'],
                            opts=['-mvp', '--detect-features', '--enable-simd'])

    def test_emit_all_features(self):
        p = shared.run_process(shared.WASM_OPT +
                               ['--emit-target-features', '-all', '-o', '-'],
                               input="(module)", check=False,
                               capture_output=True, decode_output=False)
        self.assertEqual(p.returncode, 0)
        p2 = shared.run_process(shared.WASM_OPT +
                                ['--print-features', '-o', os.devnull],
                                input=p.stdout, check=False,
                                capture_output=True)
        self.assertEqual(p2.returncode, 0)
        self.assertEqual([
            '--enable-threads',
            '--enable-mutable-globals',
            '--enable-nontrapping-float-to-int',
            '--enable-simd',
            '--enable-bulk-memory',
            '--enable-sign-ext',
            '--enable-exception-handling',
            '--enable-tail-call',
            '--enable-reference-types',
            '--enable-multivalue',
            '--enable-gc',
            '--enable-memory64',
            '--enable-relaxed-simd',
            '--enable-extended-const',
            '--enable-strings',
            '--enable-multimemory',
            '--enable-stack-switching',
            '--enable-shared-everything',
            '--enable-fp16',
            '--enable-bulk-memory-opt',
        ], p2.stdout.splitlines())
