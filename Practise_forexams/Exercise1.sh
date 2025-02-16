#!/bin/bash

dir="${1:-.}"

largest_file=$(find "$dir" -type f -exec du -b {} + | sort -nr | head -n 1)

if [ -n "$largest_file" ]; then
  echo "The largest file is: $largest_file "
 else
echo "Error, no files found in catalog $dir. "
fi
