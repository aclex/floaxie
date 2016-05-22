#!/bin/sh

checkside() {
	echo "scale=400;ibase=16;bs=$2;ibase=A;pow=$3;dpow=$1;estimate(dpow,bs,pow);" | bc -l analyze.bc
}

i=0
while read -r decpower base power; do
	i=$(($i+1))
#	echo "decpower: $decpower"
#	echo "base: $base"
#	echo "power: $power"
#	echo $(($i%4))
	checkside $decpower $base $power
	if [ $(($i%4)) -eq 0 ]; then
		echo ""
	fi
done

