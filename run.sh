#!/bin/bash

# Navigate to the vm directory
cd "$(dirname "$0")"

# Find all .c files in the current directory and subdirectories
C_FILES=$(find . -name "*.c")

# Create a string of all .c files for compilation
C_FILES_STRING=$(echo $C_FILES | tr '\n' ' ')

# Name of the output executable
EXECUTABLE="vm"

# Compile the project
clang $C_FILES_STRING -o $EXECUTABLE

# Check if compilation was successful
if [ $? -eq 0 ]; then
    ./$EXECUTABLE
else
    echo "Compilation failed. Please check your code for errors."
fi
