#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

compsum=0
perfsum=0
for(( i = 0; i < 5; i++ ))
do
    out=`make clean`
    sec=0
    if [ $1 == "u" ]; then
	   sec=`/usr/bin/time --verbose make 2>&1  | grep "User" | awk '{print $4}'`
       perfsec=`/usr/bin/time --verbose vpr iir1.map4.latren.net k4-n10.xml place.out route.out -nodisp -place_only -seed 0 2>&1 | grep "User time" | awk '{print $4}'`
       echo "$perfsec"
       perfsum=`python -c "print $perfsum+$perfsec"`
    else
	#/usr/bin/time --verbose make 2>&1 | grep "User time" | awk '{print $4}'
	   x=`/usr/bin/time --verbose make -j $1 2>&1  | grep "Elapsed" | awk '{print $8}'`
	   #echo "$x"
	   IFS=: read min sec <<< "$x"
    fi
    echo "$sec"
    compsum=`python -c "print $compsum+$sec"`
done
echo "---------"
echo "Average Compilation Time: "
echo `python -c "print $compsum/5"`
echo "Average Performance Time: "
echo `python -c "print $perfsum/5"`
echo "---------"
#ls -l vpr | awk '{print $5}'