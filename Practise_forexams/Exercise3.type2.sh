#!/bin/bash

Source_dir="os/exams"
Dest_dir="os/all-exams"

mkdir -p "$Dest_dir"

find "$Sourde_dir" -type f -name "exam.md" | while read file; do

  dir=$(dirname "$file")

 year=$(basename "$dir" | cut -d '/' -f3)
 month=$(basename "$dir" | cut -d '/' -f4)
 ae=$(basename "$dir" | cut -d "-" -f1)

new_filename="${year}-${month}-${ae}-exam.md"
mv "$file" "$Dest_dir/$new_filename"

echo "Moved"

done

