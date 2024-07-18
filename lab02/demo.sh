#!/usr/bin/env bash

# Replace <your-student-id> with your actual student ID
id=$1

# Download the challenge file
curl -o "$1.bin" "https://inp.zoolab.org/binflag/challenge?id=$id"

# Decode the flag using your program from (2)
./sol "$1.bin" > flag.txt 2>/dev/null
# ./sol.py "$1.bin" > flag.txt 2>/dev/null

# Read the flag from the file
flag=$(cat flag.txt)

# Verify the flag
curl "https://inp.zoolab.org/binflag/verify?v=$flag"

# Clean up
rm "$1.bin" flag.txt
