#!/bin/bash

# Script Name: start.bash
# Purpose: Launch all executable files in a specified directory in the background.
# Usage: start.bash <directory>
# Author: LORTET Cesar
# License: MIT

# --- Script starts ---

# Priority levels based on RFC 5424
INFO="[INFO]"
ERROR="[ERROR]"

# Validate input: Check if a directory argument is provided
if [ -z "$1" ]; then
  echo "$ERROR No directory specified. Usage: $0 <directory>" >&2
  sleep infinity
fi

# Set the directory to the argument provided
DIRECTORY="$1"

# Validate input: Check if the directory exists
if [ ! -d "$DIRECTORY" ]; then
  echo "$ERROR The directory '$DIRECTORY' does not exist." >&2
  exit 1
fi

# Iterate over files in the directory
for FILE in "$DIRECTORY"/*; do
  # Check if the file is executable and a regular file
  if [ -x "$FILE" ] && [ -f "$FILE" ]; then
    echo "$INFO Launching '$FILE' in the background..."
    "$FILE" & # Launch the file in the background
  else
    echo "$INFO Skipping '$FILE': Not an executable file."
  fi
done

# Final message indicating script completion
echo "$INFO All executable files in '$DIRECTORY' have been processed."

sleep infinity
