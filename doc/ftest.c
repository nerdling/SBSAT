/*
 * this demonstrates the difference in fpoint handling
 * and therefore different number of backtracks
 * that we might be getting on different architectures
 * and different Optimization levels
 *
 * Examples:
 *  linux (gcc 3.0, 2.95.3, 2.96, 2.91.66)
 *                with     -O3  result: Not Equal
 *                without  -O3  result: Equal
 *  Sparc solaris (gcc 2.95.3, 3.0.2, CC),  Mac OS X (gcc 3.0)
 *                with     -O3  result: Equal
 *                without  -O3  result: Equal
 */

#include <stdio.h>

double f1=51.99999999999999289457264239899814128875732421875000, 
       f2=17.63241479992457172443209856282919645309448242187500, 
       f3=0;
double g1=51.99999999999998578914528479799628257751464843750000, 
       g2=17.63241479992457527714577736333012580871582031250000,
       g3=0;

int
main()
{
  f3 = f1 * f2;
  g3 = g1 * g2;
  if (f3 == g3) printf("Equal\n"); else printf("Not Equal\n");
}


