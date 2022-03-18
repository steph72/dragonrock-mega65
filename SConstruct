import os

env = Environment(
    ENV={'PATH': os.environ['PATH']},
    CPPPATH='mega65-libc/cc65/include',
    CC='cl65')

test = env.Program('bin/drock.c64', [
    Glob("src/*.c"),
    'mega65-libc/cc65/src/memory.c',
    # 'mega65-libc/cc65/src/fcio.c',
], CCFLAGS='-DDEBUG -Or --cpu 65c02')

buildDiscAction = Action('tools/buildDisc.sh')
env.AddPostAction(test, buildDiscAction)
