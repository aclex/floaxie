#!/bin/sh

checkside() {
	echo $(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print analyze(dpow,bs,pow);" | bc -l analyze.bc)
}

while read -r decpower base power; do
	echo "decpower: $decpower"
#	echo "base: $base"
#	echo "power: $power"
	checkside $decpower $base $power
#	result=$?
#	echo "$result"
done

