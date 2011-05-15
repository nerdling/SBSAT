#!/usr/bin/env python
import sys
from optparse import OptionParser 

'''
Please note that UNSAT version seems to be harder than SAT.

Disclaimer: no formal analysis was done to verify SAT and UNSAT
This means that for some n SAT might return UNSAT and UNSAT might return SAT problem.
Please use the solver to verify UN/SAT.

I have tested n for mulple of 10 starting 20.
SAT instances seem to have at most 2 solutions.
'''

def slider4(size, sat = True):
    assert(size >= 20)
    s = size/20
    output = []

    output.append("p bdd %d %d" % (size, size))
    output.append("; automatically generated SAT slider2 with n=%d " % size)
    output.append("; Disclaimer: no formal analysis was done to verify SAT and UNSAT")
    output.append("#define add_state1(1, 2, 3, 5, 4, 6)")
    output.append("#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))")

    items = [1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2]
    for i in range(size/2):
        output.append('add_state1({0}, {1}, {2}, {3}, {4}, {5})'.format(*map(lambda x: x+i, items)))

    output.append("#define add_state2(1, 2, 3, 4, 5, 6)")
    output.append("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")
    output.append("#define add_state3(1, 2, 3, 4, 5, 6)")
    output.append("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")

    items = [1, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2]
    row = 2
    for i in range(size-size/2):
        output.append('add_state{0}({1}, {2}, {3}, {4}, {5}, {7})'.format(row, *map(lambda x: x+i, items)))
        row = 3 if row == 2 else 2

    return '\n'.join(output)

def slider4_unsat(out_type, size):
    # 80: 1675229 124.090s 
    # {1, 11,  9, 27, 39, 40 }
    # {1,  7, 10, 21, 33, 35, 40}
    # int start1[6] = {1, 17-6, 15-6, 24+3, 33+6, size/2};
    # int start2[7] = {1, 12-5, 16-6, 18+3, 27+6, 29+6, size/2};

    # {1, 11,  9, 27, 39, 40 }
    # {1,  7, 10, 23, 31, 35, 40}
    # int start1[6] = {1, 17-6, 15-6, 24+3, 33+6, size/2};
    # int start2[7] = {1, 12-5, 16-6, 18+5, 27+4, 29+6, size/2};
    #
    # good one for 80 and more unsat
    # int s=size/20;
    # int start1[6] = {1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2};
    # int start2[7] = {1, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2};
    #
    # UNSAT

    # fprintf(stderr, "{%d, %d, %d, %d, %d, %d}\n", start1[0], start1[1], start1[2], start1[3], start1[4], start1[5]);
    # fprintf(stderr, "{%d, %d, %d, %d, %d, %d, %d}\n", start2[0], start2[1], start2[2], start2[3], start2[4], start2[5], start2[6]);
    print "p bdd {0:d), {0:d)".format(size)
    print("; automatically generated UNSAT slider4 with size={0:d}".format(size))
    print("; Disclaimer: no formal analysis was done to verify SAT and UNSAT")

    #first function
    print("#define add_state1(1, 2, 3, 4, 5, 6)")
    print("#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))")
    
    for i in range(0, size/2):
        print "add_state1{0}".format(start1)
        for item in start1:
            start1[item] += 1
    
    #Second and Third Function
    print("#define add_state2(1, 2, 3, 4, 5, 6, 7)")
    print("#equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2)))")
    print("#define add_state3(1, 2, 3, 4, 5, 6, 7)")
    print("#not(equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2))))")
    for i in range(0, size-size/2):
        print "add_state{0}".format([i%2+2] + start2)
        for item in range(len(start2)):
            start2[item] += 1


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
    
    print slider4(size, sat)

if __name__ == "__main__":
    main()

