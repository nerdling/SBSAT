
import os
env = Environment()

conf = Configure(env)
if not conf.CheckCXX():
    print('!! Your compiler and/or environment is not correctly configured.')
    Exit(0)
 
if not conf.CheckFunc('printf'):
    print('!! Your compiler and/or environment is not correctly configured.')
    Exit(0)

headers = [ 'fcntl.h',
            'inttypes.h',
            'limits.h',
            'memory.h',
            'stddef.h','stdint.h','stdlib.h',
            'string.h','strings.h',
            'sys/param.h','sys/time.h',
            'termios.h',
            'unistd.h',
            'signal.h',
            'assert.h',
            'iostream.h', 'fstream.h', 'iostream', 'fstream',
            'sys/types.h', 'sys/times.h', 'sys/resource.h', 'sys/wait.h',
            'sys/stat.h', 'ctype.h', 'math.h', 'regex.h',
            'ncurses/termcap.h',
            'termcap.h'
          ]

for h in headers:
    if not conf.CheckHeader(h):
        print "You need {0} to compile this program".format(h)
        Exit(1)

#perform checks here
env = conf.Finish()

env.Program(target='helloworld', source=["helloworld.c"])