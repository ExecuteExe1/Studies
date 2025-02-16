#!/bin/bash

if [ -z "$1" ]; then
	argument1="usernames.txt"
else
	argument1="$1"
fi



usernames=$(cat "$argument1")

last_login(){
	local user=$1
	echo "$user"
	echo  "###########"
	echo "Last login date(s):"
 	last -n 20 "$user" | awk '{print $4, $5, $6, $7}' | grep -E "^[A-Za-z]{3}" | uniq | head -n 20
	echo "......."
	echo "Last login IP(s)/hostname(s):"
	last -n 20 "$user" | awk '{print $3}' | grep -Ev "^[A-Za-z]{3}$" | uniq | head -n 20
	echo "......."
	echo ""

}

for user in $usernames; do
	last_login "$user"
done
