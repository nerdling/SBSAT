
import os
env = Environment()

conf = Configure(env)

#uhmm
def checkThings(headers, funcs):
    if not conf.CheckCXX():
        print('!! Your compiler and/or environment is not correctly configured.')
        Exit(0)
    for h in headers:
        if not conf.CheckHeader(h):
            print "You need {0} to compile this program".format(h)
            Exit(1)
    for f in funcs:
        if not conf.CheckFunc(f):
            print('!! Your compiler and/or environment is not correctly configured.')
            print('Function {0} no can haz').format(f)
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
funcs =   [
          'printf', 'string'
          ]

#checkThings(headers, funcs)

env = conf.Finish()

#and now the real work begins
env.Program(target='sbsat', source=["src/sbsat.cc"])
