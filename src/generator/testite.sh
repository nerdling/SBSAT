#!/bin/sh 

RET=0
PARAMS=" --debug 1  -L 0 --debug-dev stdout --backjumping 0 -All 0 -In 1 --backtracks-per-report 10000 "
#PARAMS=" --debug 1  -L 0 --backjumping 0 -All 1 -In 1 --smurfs-share-paths 0 --backtracks-per-report 10000 file.xor"

while test $RET -eq 0
do
  D=`date +%H%M%S`
  FILENAME=file.$D.xor
  #./gentest 4 4 4 2 | tee file.xor | ../ite --debug 0 # --num-buckets 3 --size-buckets 2 # -L 0 --backjumping 0 --autarky 1
  #./gentest 100 30 30 4 > $FILENAME
  ./gentest 50 60 10 4 > $FILENAME
  #./gentest 28 40 18 4 > $FILENAME
  ../ite $PARAMS --break-xors 1 --smurfs-share-paths 0 $FILENAME > result.txt 
  cat result.txt 
  AU=`cat result.txt | awk -F\  '{ print $3 }' `
  RET=$?
  if test $RET -ne 0 ; then 
    break
  fi
  ../ite $PARAMS --break-xors 0  --smurfs-share-paths 0 $FILENAME > result.txt
  NAU=`cat result.txt | awk -F\  '{ print $3 }' `
  cat result.txt 
  RET=$?
  if test $AU = $NAU ; then
    echo "good - $AU - $NAU"
    rm $FILENAME
  else
    echo "bad - $AU - $NAU $FILENAME"
    cp $FILENAME bad
    #RET=1
  fi
done

echo Failed with error $RET
