#!/bin/sh 

RET=0
PARAMS=" --debug 1  -L 0 --debug-dev stdout --backjumping 0 -All 0 -In 1 --backtracks-per-report 10000 "
#PARAMS=" --debug 1  -L 0 --backjumping 0 -All 1 -In 1 --smurfs-share-paths 0 --backtracks-per-report 10000 file.xor"

#GENTEST="4 4 4 2"
#GENTEST="100 30 30 4"
#GENTEST="50 60 10 4"
GENTEST="xor 28 40 18 4"
#GENTEST="40 20 20 20"

while test $RET -eq 0
do
  D=`date +%H%M%S`
  FILENAME=file.$D.xor
  ./gentest $GENTEST > $FILENAME
  # | tee file.xor | ../sbsat -H l --debug 1  -L 0 --backjumping 0 
  ../sbsat $PARAMS --break-xors 1 $FILENAME > result.txt 
  cat result.txt 
  AU=`cat result.txt | awk -F\  '{ print $3 }' `
  RET=$?
  if test $RET -ne 0 ; then 
    break
  fi
  ../sbsat $PARAMS --break-xors 0 $FILENAME > result.txt
  cat result.txt 
  NAU=`cat result.txt | awk -F\  '{ print $3 }' `
  RET=$?
  if test $RET -ne 0 ; then 
    break
  fi
  if test $AU = $NAU ; then
    echo "good - $AU - $NAU"
    #rm $FILENAME
  else
    echo "bad - $AU - $NAU $FILENAME"
    cp $FILENAME bad
    #RET=1
  fi
done

echo Failed with error $RET
