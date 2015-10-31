#!/bin/bash
sum=0
for((i=0;i<5;i++))
do
	make clean
	sec=0
#	if [$1 == 0]; then
#		sec=`/usr/bin/time --verbose make -j $2 2>&1  | grep "User" | awk '{print $4}'`
#	else
		#/usr/bin/time --verbose make 2>&1 | grep "User time" | awk '{print $4}'
		x=`/usr/bin/time --verbose make -j $1 2>&1  | grep "Elapsed" | awk '{print $8}'`
		echo "$x"
		IFS=: read min sec <<< "$x"
		echo "$sec"
 #   fi
	sum=`python -c "print $sum+$sec"`
done
result=22.5
var=$(awk 'BEGIN{ print "'$sum'"<"'$result'" }')

if [ $var -eq 1 ]
then
echo "---------"
echo `python -c "print $sum/5"`
echo "---------"
else
	./script.sh 2
fi

#x=`/usr/bin/time --verbose vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>&1 >/dev/null | grep "User time" | awk '{print $4}'`