#!/usr/bin/env python

import os, sys, subprocess, difflib

print '[ processing and updating testcases... ]\n'

for asm in sorted(os.listdir('test')):
  if asm.endswith('.asm.js'):
    wasm = asm.replace('.asm.js', '.fromasm')
    for precise in [1, 0]:
      cmd = [os.path.join('bin', 'asm2wasm'), os.path.join('test', asm)]
      if not precise:
        cmd += ['--imprecise']
        wasm += '.imprecise'
      print '..', asm, wasm
      print '    ', ' '.join(cmd)
      actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
      open(os.path.join('test', wasm), 'w').write(actual)

for wasm in sorted(os.listdir('test')):
  if wasm.endswith('.wast') and os.path.basename(wasm) not in ['kitchen_sink.wast']: # i64s in kitchen_sink
    print '..', wasm
    asm = wasm.replace('.wast', '.2asm.js')
    actual, err = subprocess.Popen([os.path.join('bin', 'wasm2asm'), os.path.join('test', wasm)], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', asm), 'w').write(actual)

for dot_s_dir in ['dot_s', 'llvm_autogenerated']:
  for s in sorted(os.listdir(os.path.join('test', dot_s_dir))):
    if not s.endswith('.s'): continue
    print '..', s
    wasm = s.replace('.s', '.wast')
    full = os.path.join('test', dot_s_dir, s)
    stack_alloc = ['--allocate-stack=1024'] if dot_s_dir == 'llvm_autogenerated' else []
    cmd = [os.path.join('bin', 's2wasm'), full, '--emscripten-glue'] + stack_alloc
    if s.startswith('start_'):
      cmd.append('--start')
    actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    assert err == '', 'bad err:' + err

    expected_file = os.path.join('test', dot_s_dir, wasm)
    open(expected_file, 'w').write(actual)

'''
for wasm in ['address.wast']:#os.listdir(os.path.join('test', 'spec')):
  if wasm.endswith('.wast'):
    print '..', wasm
    asm = wasm.replace('.wast', '.2asm.js')
    proc = subprocess.Popen([os.path.join('bin', 'wasm2asm'), os.path.join('test', 'spec', wasm)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    actual, err = proc.communicate()
    assert proc.returncode == 0, err
    assert err == '', 'bad err:' + err
    expected_file = os.path.join('test', asm)
    open(expected_file, 'w').write(actual)
'''

for t in sorted(os.listdir(os.path.join('test', 'print'))):
  if t.endswith('.wast'):
    print '..', t
    wasm = os.path.basename(t).replace('.wast', '')
    cmd = [os.path.join('bin', 'binaryen-shell'), os.path.join('test', 'print', t), '--print']
    print '    ', ' '.join(cmd)
    actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', 'print', wasm + '.txt'), 'w').write(actual)
    cmd = [os.path.join('bin', 'binaryen-shell'), os.path.join('test', 'print', t), '--print-minified']
    print '    ', ' '.join(cmd)
    actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', 'print', wasm + '.minified.txt'), 'w').write(actual)

for t in sorted(os.listdir(os.path.join('test', 'passes'))):
  if t.endswith('.wast'):
    print '..', t
    passname = os.path.basename(t).replace('.wast', '')
    cmd = [os.path.join('bin', 'binaryen-shell'), ('--' + passname if passname != 'O' else '-O'), os.path.join('test', 'passes', t), '--print']
    print '    ', ' '.join(cmd)
    actual, err = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    open(os.path.join('test', 'passes', passname + '.txt'), 'w').write(actual)

print '\n[ checking binary format testcases... ]\n'

for wast in sorted(os.listdir('test')):
  if wast.endswith('.wast') and not wast in []: # blacklist some known failures
    cmd = [os.path.join('bin', 'wasm-as'), os.path.join('test', wast), '-o', 'a.wasm']
    print ' '.join(cmd)
    if os.path.exists('a.wasm'): os.unlink('a.wasm')
    subprocess.check_call(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert os.path.exists('a.wasm')

    cmd = [os.path.join('bin', 'wasm-dis'), 'a.wasm', '-o', 'a.wast']
    print ' '.join(cmd)
    if os.path.exists('a.wast'): os.unlink('a.wast')
    subprocess.check_call(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert os.path.exists('a.wast')
    actual = open('a.wast').read()
    open(os.path.join('test', wast + '.fromBinary'), 'w').write(actual)

print '\n[ checking example testcases... ]\n'

for t in sorted(os.listdir(os.path.join('test', 'example'))):
  output_file = os.path.join('bin', 'example')
  cmd = ['-Isrc', '-g', '-lasmjs', '-lsupport', '-Llib/.', '-pthread', '-o', output_file]
  if t.endswith('.cpp'):
    cmd = [os.path.join('test', 'example', t),
           os.path.join('src', 'pass.cpp'),
           os.path.join('src', 'wasm.cpp'),
           os.path.join('src', 'passes', 'Print.cpp')] + cmd
  elif t.endswith('.c'):
    # build the C file separately
    extra = [os.environ.get('CC') or 'gcc',
             os.path.join('test', 'example', t), '-c', '-o', 'example.o',
             '-Isrc', '-g', '-Llib/.', '-pthread']
    print ' '.join(extra)
    subprocess.check_call(extra)
    # Link against the binaryen C library DSO, using an executable-relative rpath
    cmd = ['example.o', '-lbinaryen-c'] + cmd + ['-Wl,-rpath=$ORIGIN/../lib']
  else:
    continue
  if os.environ.get('COMPILER_FLAGS'):
    for f in os.environ.get('COMPILER_FLAGS').split(' '):
      cmd.append(f)
  cmd = [os.environ.get('CXX') or 'g++', '-std=c++11'] + cmd
  print ' '.join(cmd)
  try:
    subprocess.check_call(cmd)
    actual = subprocess.Popen([output_file], stdout=subprocess.PIPE).communicate()[0]
    open(os.path.join('test', 'example', '.'.join(t.split('.')[:-1]) + '.txt'), 'w').write(actual)
  finally:
    os.remove(output_file)


print '\n[ success! ]'
