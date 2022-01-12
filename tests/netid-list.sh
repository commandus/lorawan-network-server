#!/bin/bash
input="netid-list.txt"
while IFS= read -r id
do
	read -r addrline
	addrline=`echo "$addrline" | tr '[:upper:]' '[:lower:]'`
	addrmin=${addrline%-*}
	addrmax=${addrline#*-}
	read -r name
	read -r region
	read -r dummy
	crline=`../lsnetid $id`
	arr_crline=(${crline//\t/ })
	echo "$id $addrmin $addrmax ${arr_crline[4]} ${arr_crline[5]}"
	if [ $arr_crline[4] = $addrmin ]; then
		echo "Bad lower address"
	fi
	if [ $arr_crline[5] = $addrmax ]; then
		echo "Bad lower address"
	fi


done < "$input"