# Copyright 2024 WebAssembly Community Group participants
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

'''
Given two files that are the output of --metrics, show a nice diff of them. For
example,

$ wasm-opt a.wasm --metrics > a
$ wasm-opt b.wasm --metrics > b
$ python scripts/diff_metrics a b
Const      :     +1613
RefNull    :      +114
StringConst:        -1
StructNew  :        -2
[exports]  :       +70
[funcs]    :      +132
[globals]  :        +7
[imports]  :        +4
[memories] :        +1
[tables]   :        +1
[tags]     :        +1
[total]    :     +9773
[vars]     :      +554

Only differences are shown, so lines that would be "0" are elided.
'''

import os, sys

def parse(filename):
  ret = {}
  for line in open(filename).readlines():
    if ' : ' in line:
      item, count = line.split(' : ')
      ret[item.strip()] = int(count)
  return ret

a = parse(sys.argv[1])
b = parse(sys.argv[2])

items = set(list(a.keys()) + list(a.keys()))

longest_name = max([len(item) for item in items])

for item in sorted(list(items)):
  prefix = item + (' ' * (longest_name - len(item))) + ': '

  if item in a and item not in b:
    value = f'-{a[item]}'
  elif item not in a and item in b:
    value = f'+{a[item]}'
  else:
    diff = b[item] - a[item]
    # ensure a + or - prefix
    value = str(diff)
    if diff > 0:
      value = '+' + value

  # left-justify
  value = '{:>8}'.format(value)
  print(f'{prefix} {value}')

