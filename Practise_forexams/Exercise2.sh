#!/bin/bash

mkdir -p os/0209-os-exams/

for year in $(seq 1973 2022); do
 for month in 02 09; do
  src_path="exams/$year/$month/os/results.md"

if [ -f "$scr_path" ]; then
  dest_file="os/0209-os-exams/${year}-${month}-results.md"

 mv "$src_path" "$dest_file"
 echo "Moved"
 fi
done
done

echo "Proccess complete"
