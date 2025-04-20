#!/bin/bash
gcc main.c -o main

comp=0
pids=()
for i in {0..9}
do
    ./main test &
    pids+=($!)
done

sleep 300

for pid in ${pids[@]}
do
    kill -SIGINT $pid
    if [[ $? -eq 0 ]]
    then
        ((comp++))
    fi
done

echo "Expected: 10" >> result.txt
echo "Actual: " $comp >> result.txt
cat stat >> result.txt
rm stat
