#!/bin/sh 

RET=0
PARAMS=" --debug 1 -gauss 1 --debug-dev stdout --backtracks-per-report 10000 -Ex 0"
#PARAMS=" --debug 1  -L 0 --debug-dev stdout --backjumping 0 -All 0 -In 1 --backtracks-per-report 10000 "
#PARAMS=" --debug 1  -L 0 --backjumping 0 -All 1 -In 1 --smurfs-share-paths 0 --backtracks-per-report 10000 file.xor"

#GENTEST="4 4 4 2"
#GENTEST="xor 100 30 30 4"
#GENTEST="xor 50 60 10 4"
#GENTEST="xor 28 40 18 4"
#GENTEST="xor 48 40 18 4"
#GENTEST="xor 14 25 10 3"
GENTEST="xor 35 35 5 2"
#GENTEST="40 20 20 20"
c=0
while test $RET -eq 0
do
  c=$(($c+1))
  D=`date +%H%M%S`$c
  FILENAME=file.$D.xor
  ./gentest $GENTEST > $FILENAME
  # | tee file.xor | ../sbsat -H l --debug 1  -L 0 --backjumping 0 
  ../sbsat $PARAMS --break-xors 1 $FILENAME > result0.txt
  cat result0.txt 
  AU=`cat result0.txt | awk -F\  '{ print $3 }' `
  RET=$?
  if test $RET -ne 0 ; then 
    break
  fi
  ../sbsat --debug 1 --break-xors 1 $FILENAME > result1.txt
  cat result1.txt 
  NAU=`cat result1.txt | awk -F\  '{ print $3 }' `
  RET=$?
  if test $RET -ne 0 ; then 
    break
  fi

#  ../sbsat $PARAMS --break-xors 0 $FILENAME > result.txt
#  cat result.txt 
#  NAU=`cat result.txt | awk -F\  '{ print $3 }' `
#  RET=$?
#  if test $RET -ne 0 ; then 
#    break
#  fi
#  if test $AU = $NAU ; then
#    echo "good - $AU - $NAU"
#    rm $FILENAME
#  else
#    echo "bad - $AU - $NAU $FILENAME"
#    cp $FILENAME bad
    #RET=1
#  fi
done

echo Failed with error $RET
