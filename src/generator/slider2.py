#!/usr/bin/env python
from optparse import OptionParser 

'''
Please note that UNSAT version seems to be harder than SAT.

Disclaimer: no formal analysis was done to verify SAT and UNSAT
This means that for some n SAT might return UNSAT and UNSAT might return SAT.
Please use the solver to verify UN/SAT.

I have tested n for multiples of 10 starting 20.
SAT instances seem to have at most 2 solutions.
'''

def slider4(size, sat = 1):
    assert(size >= 20)
    s = size/20
    type = '' if sat else 'UN'
    order = [5,4] if sat else [4,5]
    output = []

    output.append("p bdd %d %d" % (size, size))
    output.append("; automatically generated %sSAT slider2 with n=%d " % (type, size))
    output.append("; Disclaimer: no formal analysis was done to verify SAT and UNSAT")
    output.append("#define add_state1(1, 2, 3, %d, %d, 6)" % (order[0], order[1]))
    output.append("#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))")

    items = [1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2]
    for i in range(size/2):
        output.append('add_state1({0}, {1}, {2}, {3}, {4}, {5})'.format(*[x+i for x in items]))

    output.append("#define add_state2(1, 2, 3, 4, 5, 6)")
    output.append("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")
    output.append("#define add_state3(1, 2, 3, 4, 5, 6)")
    output.append("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")

    items = [1, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2]
    row = 2
    for i in range(size-size/2):
        output.append('add_state{0}({1}, {2}, {3}, {4}, {5}, {7})'.format(row, *[x+i for x in items]))
        row = 3 if row == 2 else 2

    print '\n'.join(output)

#main method. Option parsing
def main():
    usage = []
    usage.append("usage: %prog size sat")
    usage.append("       size - num variables")
    usage.append("       sat - 0/1 = un/sat")
    parser = OptionParser(usage='\n'.join(usage))
    
    (_, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("Unexpected quantity of variables.")
    [size, sat] = map(int, args)
    
    slider4(size, sat)

if __name__ == "__main__":
    main()

