#!/bin/bash

mkdir -p os/all-exams

for year in $(seq 1997 2019); do
 for month in 02 06 09; do
 
   src_path="os/exams/$year/$month"
  
  if [ -d "$src_path" ]; then
   for file in "$src_path"/*-exam.md; do
    if [ -f "file" ]; then
     ae=$(basename "$file" | cut -d "-" -f1)
      new_filename="${year}-${month}-${ae}-exam.md"
   
       mv "$file" "$src_path/$new_filename"

         echo "Move complete" 
      fi
    done
   fi
  done
done
