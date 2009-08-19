
import sys
from optparse import OptionParser 

def slider3(out_type, size, sat, offset):  
    if   sat == 0:
        slider3_unsat(out_type, size, offset)
    elif sat == 1:
        slider3_sat(out_type, size, offset)
    elif sat == 2:
        slider3_unsat_iscas(size, offset)
    elif sat == 3:
        slider3_unsat_trace(size, offset)
    else:
        print >> sys.stderr, 'Error: Unknown out_type: %s' % out_type
        exit(0)

'''
 * sliders
 *
 * unsat
 * (first function -- notice 1,2,3,4,5,6)
   #define add_state1(1, 2, 3, 4, 5, 6)
   #equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))
 *
 * (second/third function)
   #define add_state2(1, 2, 3, 4, 5, 6, 7)
   #equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2)))
   #define add_state3(1, 2, 3, 4, 5, 6, 7)
   #not(equ(6, xor(-1, xor(3, and(-4, 5), 4), equ(7, 2))))
 *
 * sat
 * (first function -- notice 1,2,3,5,4,6 -- 5,4 switched)
   #define add_state1(1, 2, 3, 5, 4, 6)\n");
   #equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))
 *
 * (second/third functions - notice -- they are the same)
   #define add_state2(1, 2, 3, 4, 5, 6)
   #xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))
   #define add_state3(1, 2, 3, 4, 5, 6)
   #xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))
 *
 * numbers are generated the same way for both sat and unsat
   int start1[6] = {1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2};
   int start2[7] = {1, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2};
 *
 * Please note that UNSAT version seems to be harder than SAT.
 *
 * Disclaimer: no formal analysis was done to verify SAT and UNSAT
 * This means that for some n SAT might return UNSAT and UNSAT might return SAT problem.
 * Please use the solver to verify UN/SAT.
 *
 * I have tested n for mulple of 10 starting 20.
 * SAT instances seem to have at most 2 solutions.
 *'''

def slider3_sat(out_type, size, offset):
	s = size/20
	start1 = [1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2+1+offset]
	start2 = [1, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2+1+offset]
	'''
	print >> sys.stderr, start1
	print >> sys.stderr, start2
	'''
	print "p bdd %d %d" % (size+offset,size)
	print "; automatically generated SAT slider3 with size=%d " % size
	print "; Disclaimer: no formal analysis was done to verify SAT and UNSAT"

	#first function
	print "#define add_state1(1, 2, 3, 5, 4, 6)"
	print "#equ(xor(1, and(-3, 5), nand(6, 4)), ite(2, or(5, -6), -5)))"
	for i in range(0, size/2):
		sys.stdout.write("add_state1")
		print tuple(start1)
		for item in range(len(start1)):
			start1[item] += 1

	#second and third function
	print("#define add_state2(1, 2, 3, 4, 5, 6)")
	print("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")
	print("#define add_state3(1, 2, 3, 4, 5, 6)")
	print("#xor(-1, xor(3, and(-4, 5), 4), equ(6, 2))")
	for i in range(0, size-size/2):
		test = (i%2)+2
		sys.stdout.write("add_state%d" % test)
		print tuple(start2[0:5]+start2[6:])
		for item in range(len(start2)):
			start2[item] += 1
    	
def slider3_unsat_trace(size):
	s = size/20
	start1 = [1, 2*s+3, 2*s+1, size/2-1-3*s, size/2-1, size/2+1]
	start2 = [size, 2*s-1, 2*s+2, size/2-1-4*s, size/2-1-2*s, size/2-1-s, size/2+1]
	tmpnum = 0
	
	sys.stdout.write("MODULE slider3_{0:d}_unsat\nINPUT v_{1:d}".format(size, size/2+1))
	for i in range(size/2+2,size+1):
		sys.stdout.write(", v_{0:d}".format(i))
	sys.stdout.write(";\n")
	
	print "OUTPUT MITER;"
	print "STRUCTURE"
	
	#first function
	for i in range(0,size/2):
		print "g_{0:d}  = not(v_{1:d});".format(tmpnum+1, start1[1])
		print "g_{0:d}  = not(v_{1:d});".format(tmpnum+2, start1[2])
		print "g_{0:d}  = or(g_{1:d}, g_{3:d});".format(tmpnum+3, tmpnum+1, tmpnum+2)
		print "g_{0:d}  = or(v_{1:d}d, v_{3:d});".format(tmpnum+4, start1[1], start1[2])
		print "g_{0:d}  = and(g_{1:d}, g_{3:d});".format(tmpnum+5, tmpnum+3, tmpnum+4)
		print "g_{0:d}  = not(g_{1:d});".format(tmpnum+6, tmpnum+5)
		print "g_{0:d}  = and(v_{1:d}, g_{3:d});".format(tmpnum+7, start1[4], tmpnum+5)
		print "g_{0:d}  = not(v_{1:d});".format(tmpnum+8, start1[4])
		print "g_{0:d}  = and(v_{1:d}, g_{3:d});".format(tmpnum+9, start1[4], tmpnum+6)
		print "g_{0:d} = and(g_{1:d}, g_{3:d});".format(tmpnum+10, tmpnum+8, tmpnum+1)
		print "g_{0:d} = or(g_{1:d}, g_{3:d});".format(tmpnum+11, tmpnum+9, tmpnum+10)
		print "g_{0:d} = and(v_{1:d}, g_{3:d});".format(tmpnum+12, start1[4], tmpnum+5)
		print "g_{0:d} = and(g_{1:d}, v_{3:d});".format(tmpnum+13, tmpnum+8, start1[1])
		print "g_{0:d} = or(g_{1:d}, g_{3:d});".format(tmpnum+14, tmpnum+12, tmpnum+13)
		print "g_{0:d} = and(v_{1:d}, g_{3:d});".format(tmpnum+15, start1[3], tmpnum+11)
		print "g_{0:d} = not(v_{1:d});".format(tmpnum+16, start1[3])
		print "g_{0:d} = and(g_{1:d}, g_{3:d});".format(tmpnum+17, tmpnum+16, tmpnum+14)
		print "g_{0:d} = or(g_{1:d}, g_{3:d});".format(tmpnum+18, tmpnum+15, tmpnum+17)
		print "g_{0:d} = and(v_{1:d}, g_{3:d});".format(tmpnum+19, start1[5], tmpnum+18)
		print "g_{0:d} = not(v_{1:d});".format(tmpnum+20, start1[5])
		print "g_{0:d} = and(g_{1:d}, g_{3:d});".format(tmpnum+21, tmpnum+20, tmpnum+7)
		print "v_{3:d} = or(g_{1:d}, g_{3:d});".format(start1[0], tmpnum+19, tmpnum+21)
		
		tmpnum += 21
		for item in range(len(start1)):
			start1[1]+=1
		
	#Second and Third function
	
	#xor(T, 1, 2, 3, 6, 7, or(4, 5))
	for i in range(0,size-size/2):
		if i%2:
			print "g_{0:d} = or(v_{1:d}, v_{2:d});".format( tmpnum+1, start2[3], start2[4])
			print "g_{0:d} = xor(v_{1:d}, v_{2:d}, v_{3:d}, v_{4:d}, g_{5:d});".format( tmpnum+2, start2[1], start2[2], start2[5], start2[6], tmpnum+1)
			print "v_{0:d} = not(g_{1:d});".format( start2[0], tmpnum+2)
			tmpnum += 2
		else:
			print "g_{0:d} = or(v_{1:d}, v_{2:d});".format( tmpnum+1, start2[3], start2[4])
			print "v_{0:d} = xor(v_{1:d}, v_{2:d}, v_{3:d}, v_{4:d}, g_{5:d});".format( start2[0], start2[1], start2[2], start2[5], start2[6], tmpnum+1)
			tmpnum+=1
		
		for item in range(len(start2)):
			start2[item] += 1
	
	for i in range(1, size/2+1):
		start1[0] -= 1
		start2[0] -= 1
		print "g_{0:d} = xor(v_{1:d}, v_{2:d});".format(tmpnum+i, start1[0], start2[0])
	
	sys.stdout.write("MITER = or(g_{0:d}".format(tmpnum+1))
	for i in range(2,size/2+1):
		sys.stdout.write(", g_{0:d}".format(tmpnum+i))
	sys.stdout.write(");\nfalse_value = new_int_leaf(0);\nare_equal(false_value, MITER);\nENDMODULE\n")

def slider3_unsat(out_type, size):
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
	print("; automatically generated UNSAT slider3 with size={0:d}".format(size))
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
	usage = "usage: %prog [options] size sat\n  size                  num variables\n  sat                   1/0 (un/sat)"
	parser = OptionParser(usage=usage)
	parser.add_option("-o", "--offset", type="int", nargs=1, default=0, help="sets the offset", dest="offset")
	
	(options, args) = parser.parse_args()
	if len(args) != 2:
		parser.error("Try again")
	[size, sat] = map(int, args)
	
	slider3('ite', size, sat, offset=options.offset)

if __name__ == "__main__":
    main()

