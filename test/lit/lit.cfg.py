import os
import lit.formats

config.name = "Binaryen lit tests"
config.test_format = lit.formats.ShTest()

config.suffixes = ['.wat', '.wast', '.test']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.binaryen_build_root, 'out', 'test')

config.environment = dict(os.environ)

# Replace all Binaryen tools with their absolute paths
bin_dir = os.path.join(config.binaryen_build_root, 'bin')
assert(os.path.isdir(bin_dir))

for tool_file in os.listdir(bin_dir):
    tool_path = config.binaryen_build_root + '/bin/' + tool_file
    tool = tool_file[:-4] if tool_file.endswith('.exe') else tool_file
    config.substitutions.append((tool, tool_path))

# Also make the `not` and `foreach` commands available
for tool in ('not', 'foreach'):
    tool_file = config.binaryen_src_root + '/scripts/' + tool + '.py'
    python = sys.executable.replace('\\', '/')
    config.substitutions.append((tool, python + ' ' + tool_file))

# Finds the given executable 'program' in PATH.
# Operates like the Unix tool 'which'.
# This is similar to script/test/shared.py, but does not use binaryen_root, and
# instead is tuned to jsvu's install dir.
def which(program):
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)
    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        # Prefer the path, or jsvu's install dir.
        paths = os.environ['PATH'].split(os.pathsep) + [
          os.path.expanduser('~/.jsvu/bin'),
        ]
        for path in paths:
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file
            if '.' not in fname:
                if is_exe(exe_file + '.exe'):
                    return exe_file + '.exe'
                if is_exe(exe_file + '.cmd'):
                    return exe_file + '.cmd'
                if is_exe(exe_file + '.bat'):
                    return exe_file + '.bat'

# v8 may be provided by jsvu, or it may be "d8".
V8 = os.environ.get('V8') or which('v8') or which('d8')
config.substitutions.append(('v8', V8))
