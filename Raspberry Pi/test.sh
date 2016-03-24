#!/bin/sh

for k in 0 1 2
do
	if [ $k -eq 0 ]; then
		echo "LED - OFF"
	else
		if [ $k -eq 1 ]; then
			echo "LED - ON"
		else
			echo "LED - FLASH"
		fi
	fi
	
#	for j in 16 17 18 32 33 34 48 49 50
	for j in 16 17 18
	do
		eval "./getdevval_bb $j $k"
		echo "$j $k"
		sleep 1
	done
	
	sleep 5
done

echo "Test Complete"