#!/usr/bin/env bash

USAGE="Usage: $0 [-a] [-i <id>]"
VERIFY_URL="https://inp.zoolab.org/maze/verify"
SOL_1="./sol_1"
SOL_2="./sol_2"
SOL_3="./sol_3"
SOL_4="./sol_4"

run_all=true
id=""

if [ "$#" -eq 0 ]
then
    echo "$USAGE";
    exit 1;
fi

while getopts 'ai:' flag; do
    case "${flag}" in
        a) run_all=true ;;
        i) run_all=false; id="${OPTARG}" ;;
        *) echo "$USAGE"; exit 1 ;;
    esac
done

if [ "$run_all" = true ]; then
    # Run all binaries and capture their output
    output1=$($SOL_1)
    output2=$($SOL_2)
    output3=$($SOL_3)
    output4=$($SOL_4)
else
    # Run the binary with the given ID and capture its output
    case "$id" in
        1) output1=$($SOL_1) ;;
        2) output2=$($SOL_2) ;;
        3) output3=$($SOL_3) ;;
        4) output4=$($SOL_4) ;;
        *) echo "$USAGE"; exit 1 ;;
    esac
fi

# Parse the output of each binary to extract the token
token1=$(echo "$output1" | cut -d' ' -f2- | sed 's/=/%3D/g; s/\+/%2B/g; s/\//%2F/g')
token2=$(echo "$output2" | cut -d' ' -f2- | sed 's/=/%3D/g; s/\+/%2B/g; s/\//%2F/g')
token3=$(echo "$output3" | cut -d' ' -f2- | sed 's/=/%3D/g; s/\+/%2B/g; s/\//%2F/g')
token4=$(echo "$output4" | cut -d' ' -f2- | sed 's/=/%3D/g; s/\+/%2B/g; s/\//%2F/g')

# Use the extracted token to substitute the `v` value in the curl command
curl_command_1="curl '$VERIFY_URL' --data-raw 'v=$token1' --compressed"
curl_command_2="curl '$VERIFY_URL' --data-raw 'v=$token2' --compressed"
curl_command_3="curl '$VERIFY_URL' --data-raw 'v=$token3' --compressed"
curl_command_4="curl '$VERIFY_URL' --data-raw 'v=$token4' --compressed"

# Execute the curl command with the substituted token
if [ "$run_all" = true ]; then
    eval "$curl_command_1" > verify1.txt
    eval "$curl_command_2" > verify2.txt
    eval "$curl_command_3" > verify3.txt
    eval "$curl_command_4" > verify4.txt
else
    case "$id" in
        1) eval "$curl_command_1" > verify1.txt ;;
        2) eval "$curl_command_2" > verify2.txt ;;
        3) eval "$curl_command_3" > verify3.txt ;;
        4) eval "$curl_command_4" > verify4.txt ;;
        *) echo "$USAGE"; exit 1 ;;
    esac
fi