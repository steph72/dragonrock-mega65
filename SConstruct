import os
import shutil

# Create required directories if they don't exist
for directory in ['obj', 'bin', 'disc', 'gamedata']:
    if not os.path.exists(directory):
        print(f"Creating directory: {directory}")
        os.makedirs(directory)

env = Environment(
    ENV={'PATH': os.environ['PATH']},
    CPPPATH='mega65-libc/cc65/include',
    CC='cl65')

# Configure object files to be stored in obj directory
env.VariantDir('obj', 'src', duplicate=0)

# Use objects from obj directory
src_files = ['obj/' + os.path.basename(str(f)) for f in Glob("src/*.c")]
lib_files = ['mega65-libc/cc65/src/memory.c']

# Compile the program (without running buildDisc.sh)
program = env.Program('bin/drock.c64', 
    src_files + lib_files, 
    CCFLAGS='-DDEBUG -Or --cpu 65c02')

# Create compile alias
compile_cmd = env.Alias('compile', program)
Default(compile_cmd)  # Make 'compile' the default when running just 'scons'

# Create full build command (compile + buildDisc.sh)
buildDiscAction = Action('tools/buildDisc.sh')
build_cmd = env.Command('build', program, buildDiscAction)
env.Alias('build', build_cmd)  # Allow 'scons build' to trigger full build

# Custom clean function
def clean_directories(target, source, env):
    dirs_to_clean = ['gamedata', 'bin', 'obj', 'disc']
    for dir_path in dirs_to_clean:
        if os.path.exists(dir_path):
            print(f"Cleaning directory: {dir_path}")
            for item in os.listdir(dir_path):
                item_path = os.path.join(dir_path, item)
                if os.path.isfile(item_path):
                    os.unlink(item_path)
                elif os.path.isdir(item_path):
                    shutil.rmtree(item_path)
            # Recreate the directory after cleaning
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)
    return 0

# Add a clean target
clean_cmd = env.Command('clean', None, Action(clean_directories, "Cleaning build directories..."))
env.Alias('clean', clean_cmd)

# Help text
Help("""
Available commands:
  'scons' or 'scons compile' - Compile the project only
  'scons build' - Compile the project and build the disc image
  'scons clean' - Clean gamedata, bin, obj, and disc directories
""")
