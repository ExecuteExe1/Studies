#!/bin/bash


if [ "$#" -ne 2 ]; then
 echo "Usage: $0 <usernames_file> <filename>"
 exit 1
 fi

usernames_file="$1"
filename="$2"

if [ ! -f "$usernames_file" ]; then
  echo "Error:"$usernames_file" not found"
 exit 1 
fi

while IFS= read -r username; do
 if [ -z "$username" ] || [[ "$username" == \#* ]]; then
  continue
fi

home_dir="/home/$username"

if [ ! -d "$home_dir" ]; then
 echo "Error:Home directory for user "$username"  not found"
 continue
 elif [ ! -r "$home_dir" ]; then
  echo "Error:No access to that home directory"
 else
  continue
 fi

file_path="$home_dir/$filename"
 echo ""
echo "$username"
echo "############"
 if [ -f $"file_path" ]; then
   file_size=$(stat -c%s "$file_path")
  echo " $filename: File Found ,size  $file_size bytes"
 elif [ -e "$file_path" ]; then
     echo " $filename: Not a regular file"
  else
     echo " $filename : File not found"
  fi
done < "$usernames_file"

