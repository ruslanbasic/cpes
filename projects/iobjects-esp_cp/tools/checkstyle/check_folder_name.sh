#!/bin/bash

# check folders lower case ##################################################

echo "info: checking folder names..."

exclude="^\."`
       `"(/build"`
       `"|/components/curl)"

files=$(find . -type f \( -name "*.h" -o -name "*.c" \) | egrep -v $exclude | xargs -n1 dirname | sort -u)

file_counter=0
error_counter=0

for file in $files
do
  echo "$file" | egrep -q '^[-0-9_a-z./]+$'
  if [ $? -ne 0 ]
  then echo -e "error: bad filename $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "info: each folder name must contain low case characters only\n"
     echo -e "info: $file_counter folders checked. count of errors $error_counter\n"
     exit 1
else echo -e "info: $file_counter folders checked. count of errors $error_counter\n"
fi
