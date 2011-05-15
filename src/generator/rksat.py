#!/usr/bin/env python
from optparse import OptionParser 
from sys import stderr
from time import time
import random

def rksat(n, m, k, seed = None):
    if not seed:
        seed = int(time())
    random.seed(seed)
    print >> stderr, 'Seed:', seed

    output = []
    output.append("p cnf %d %d" % (n, m))

    row = []
    for i in range(1,m*k+1):
        row.append("%d " % (random.randint(-n,n)))
        if i % k == 0:
            row.append("0")
            output.append(''.join(row))
            row = []
    print '\n'.join(output)

#main method. Option parsing
def main():
    usage = []
    usage.append("usage: %prog n m k [seed]")
    usage.append("       n - variables")
    usage.append("       m - clauses")
    usage.append("       k - variables per clause")
    parser = OptionParser(usage='\n'.join(usage))

    (_, args) = parser.parse_args()
    if not len(args) == 3 and not len(args) == 4:
        parser.error("Unexpected quantity of variables.")
    arps = map(int, args)

    rksat(*arps)

if __name__ == '__main__':
    main()
