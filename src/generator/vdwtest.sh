#!/bin/sh

K=10
L=10
NUM=1

while true ;
do
  NUM=$(($NUM+1))
  ./gentest $NUM $K $L | ../sbsat --debug 1 --comment "$NUM $K $L" -L 0 --backjumping 0
 if [ $? != 0 ] ; then
    exit 1;
 fi
done
