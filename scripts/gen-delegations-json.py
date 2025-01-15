#!/usr/bin/env python3
#
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

#
# Generates a json file that represents the contents of
# wasm-delegations-fields.def, that is, with a structural description of all the
# subclasses of Expression that make up the core of Binaryen IR. This can be
# useful to automate bindings generation.
#
# To run this, just run the script and it outputs the json to stdout. The output
# contains things like this:
#
#   {
#     id: 'Block',
#     fields: [
#       {
#         form: 'CHILD_VECTOR',
#         name: 'list',
#       },
#       {
#         form: 'SCOPE_NAME_DEF',
#         name: 'name',
#       },
#     ]
#   },
#
# This describes an Expression type "Block", whose first field "list" is a
# CHILD_VECTOR, that is, a vector of children, and whose second field "name" is
# a scope name definition. For example, consider this block:
#
#   (block $foo
#     (nop)
#     (unreachable)
#   )
#
# In the IR for this block, the field "name" will contain "foo", and the field
# "list" will contain a vector with a pointer to a Nop expression and an
# Unreachable expression. For more details on what the forms mean, see
# wasm-delegations-fields.def.
#

import os, sys

binaryen_root = os.path.dirname(os.path.dirname(__file__))
delegations_filename = os.path.join(binaryen_root, 'src', 'wasm-delegations-fields.def')

print('[')

for l in open(delegations_filename).read().splitlines():
    # We seek lines like
    #
    # DELEGATE_FIELD_FOO(bar, baz)
    #
    # which we parse into the command (FOO) and args (bar, baz).
    if not l.startswith('DELEGATE_FIELD') or '(' not in l:
        continue
    command, after = l.split('(')
    command = command[15:]
    args, _ = after.split(')')
    args = [arg.strip() for arg in args.split(',')]
    if command == 'CASE_START':
        print(f'''  {{
    id: '{args[0]}',
    fields: [''')
    elif command == 'CASE_END':
        print('    ]\n  },')
    elif len(args) == 2:
        # e.g.
        #
        # DELEGATE_FIELD_CHILD(ArrayFill, value)
        print(f'''      {{
        form: '{command}',
        name: '{args[1]}',
      }},''')
    elif len(args) == 3:
        # e.g.
        #
        # DELEGATE_FIELD_NAME_KIND(ArrayInitData, segment, ModuleItemKind::DataSegment)
        kind = args[2].split('::')[1]
        print(f'''      {{
        form: '{command}',
        name: '{args[1]}',
        kind: '{kind}',
      }},''')
    else:
        raise Exception(l)

print(']')

