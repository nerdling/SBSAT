#!/bin/sh


awk 'BEGIN{N=0}{print N" "$0; N=N+1;}' results.txt > results.dat
awk 'BEGIN{N=0}{print N" "$0; N=N+1;}' results_orig.txt > results_orig.dat

gnuplot ../plot.gpl
DIR="`pwd`"
NAME="`basename $DIR`"
mv plot.ps $NAME.ps

exit

--------------------------- plot.gpl ---------------------------------
set yrange [0:*]

plot \
        "results_orig.dat" using 1:4 t "Non-Weight J heuristic" w linespoints, \
        "results.dat" using 1:4 t "Weight Aware J Heuristic" w linespoints

#plot \
        "results.dat" using 1:5 w linespoints, \
        "results_orig.dat" using 1:5 w linespoints

#set terminal png
#set output 'plot.png'
#set size 0.5, 0.5
set terminal postscript
set output 'plot.ps'
replot
set terminal x11
set output

pause -1 "Press any key"
---------------------------------------------------------------------
# create lemmas (see crtwin.c to enable dump_lemmas at every display message)
../ite --reports 1 $datafile -H j --lemma-out-file "lemmas.txt."

# reloading lemmas
datafile=../../tests/longer_tests/dlx2_cc.trace
for i in lemma* ; do echo -n $i; ../ite --debug 1 $datafile -H jq --lemma-in-file $i; done

# and for comparison the standard heuristic
for i in lemma* ; do echo -n $i; ../ite --debug 1 $datafile -H j --lemma-in-file $i; done
----------------------------------------------------------------------
