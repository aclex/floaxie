#!/bin/sh

CALC=$(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print (bs * 2 ^ pow)" | bc -l)
TGT=$(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print (10 ^ dpow)" | bc -l)
RET=$(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print (10 ^ dpow - bs * 2 ^ pow)" | bc -l)
BRET=$(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print (10 ^ dpow - bs * 2 ^ pow) > 0" | bc -l)

echo "calculated:  $CALC"
echo "destination: $TGT"
echo "result:      $RET"
echo $BRET

