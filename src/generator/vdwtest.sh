#!/bin/sh

K=3
L=4
NUM=1

while true ;
do
  #./gentest vdw cnf $NUM $K $L | ../sbsat --debug 1 --comment "$NUM $K $L" -L 0 --backjumping 0 -All 0 -K 2.1 --backtracks-per-report 100 --max-solutions 0
  echo -n " $NUM $K $L "
  ./gentest vdw cnf $NUM $K $L > f.cnf
  ~/tmp/zchaff/zchaff f.cnf
 if [ $? != 0 ] ; then
    exit 1;
 fi
 NUM=$(($NUM+1))
done
