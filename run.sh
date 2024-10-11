#!/bin/bash

# Navigate to the vm directory
cd "$(dirname "$0")"

# Find all .c files in the current directory and subdirectories
C_FILES=$(find . -name "*.c")

# Create a string of all .c files for compilation
C_FILES_STRING=$(echo $C_FILES | tr '\n' ' ')

# Name of the output executable
EXECUTABLE="vm"

# Compile the project with all warnings and debugging symbols
COMPILE_OUTPUT=$(clang -Wall -Wextra -g $C_FILES_STRING -o $EXECUTABLE 2>&1)

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running the program..."
    # Run the program and capture its output and any errors
    OUTPUT=$(./$EXECUTABLE 2>&1)
    EXIT_CODE=$?
    echo "$OUTPUT"

    if [ $EXIT_CODE -ne 0 ]; then
        echo "Program crashed with exit code $EXIT_CODE"
        echo "Running the program with lldb for more information:"
        lldb -o "run" -o "bt" -o "quit" ./$EXECUTABLE
    fi
else
    echo "Compilation failed. Error output:"
    echo "$COMPILE_OUTPUT"
fi
