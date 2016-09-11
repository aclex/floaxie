#!/bin/sh

checkside() {
	return $(echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;print (10 ^ dpow - bs * 2 ^ pow) >= 0" | bc -l)
}

while read -r decpower base power; do
#	echo "decpower: $decpower"
#	echo "base: $base"
#	echo "power: $power"
	checkside $decpower $base $power
	result=$?
	echo "$result"
done

