#!/bin/bash

# Check if path and pattern are provided
if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "Usage: $0 <path> <pattern> [threads]"
  exit 1
fi

# Assign path and pattern to variables
search_path="$1"
search_pattern="$2"
threads="${3:-4}"  # Default to 4 threads if not provided

# Run alien_find and store the output in a variable
file_list=$(alien_find "$search_path" "$search_pattern")

# Check if alien_find returned any results
if [ -z "$file_list" ]; then
  echo "No files found matching the pattern '$search_pattern' in '$search_path'."
  exit 1
fi

# Function to copy a file and create directories
copy_file() {
  local file_path="$1"
  local dir_path

  dir_path=$(dirname "$file_path")
  
  # Create the corresponding directory structure in the current directory
  mkdir -p "./$dir_path"
  
  # Copy the file to the corresponding directory
  alien_cp alien://$file_path file:./$dir_path
}

# Export the function and variables to be used by xargs
export -f copy_file

# Use xargs to run the copy_file function in parallel
echo "$file_list" | xargs -I {} -n 1 -P "$threads" bash -c 'copy_file "$@"' _ {}

echo "Files have been successfully copied."
