#!/bin/bash

# Importing students' answers
students_counter=0
empty_answers="---------------------------"

INPUT=$1
OUTPUT=$2

echo "i : $INPUT"
echo "o : $OUTPUT"

# Writing 
rm -rf $OUTPUT
touch $OUTPUT
while read line; do
	IFS=';' read -a tokens <<< "$line"
	number="${tokens[0]}"
	surname="${tokens[2]}"
	name="${tokens[3]}"
	echo -e "${number}\t\t${empty_answers}\t\t${surname}\t${name}" >> $OUTPUT
done < $INPUT

#if [[ "$1" == "-l" ]]; then
#	tail -5 $OUTPUT
#fi
