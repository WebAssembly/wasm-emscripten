import os
import unittest
from scripts.test.shared import WASM_OPT, run_process, options

'''Test that MemoryPacking correctly respects the web limitations by not
generating more than 100K data segments'''

class MemoryPackingTest(unittest.TestCase):
  def test_large_segment(self):
    data = '"' + (('A' + ('\\00' * 9)) * 100001) + '"'
    module = '''
    (module
     (memory 256 256)
     (data (i32.const 0) %s)
    )
    ''' % data
    opts = ['--memory-packing', '--disable-bulk-memory', '--print',
            '-o', os.devnull]
    p = run_process(WASM_OPT + opts, input=module, check=False,
                    capture_output=True)
    output = [
        '(data (i32.const 999970) "A")',
        '(data (i32.const 999980) "A")',
        '(data (i32.const 999990) "A' + ('\\00' * 9) + 'A")'
    ]
    self.assertEqual(p.returncode, 0)
    for line in output:
      self.assertIn(line, p.stdout)
