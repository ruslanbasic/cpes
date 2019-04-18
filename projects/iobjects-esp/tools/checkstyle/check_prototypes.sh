#!/bin/bash

# check prototypes lower case #################################################

echo "info: checking functions prototypes..."

exclude="^\."`
       `"(/build"`
       `"|/eclipse"`
       `"|/components/esp-adf"`
       `"|/components/edbg"`
       `"|/components/bme280"`
       `"|/components/curl"`
       `"|/components/libfvad"`
       `"|/components/kws_guess"`
       `"|/components/libwebsockets"`
       `"|/components/mbedtls"`
       `"|/components/stpm3x"`
       `"|/components/unity"`
       `"|/components/zlib)"

files=$(find . -type f \( -name "*.h" \) | egrep -v $exclude)

file_counter=0
error_counter=0

for file in $files
do
  prototypes=`ctags -x --c-kinds=pf "$file" | awk '{print $1}' | egrep '[A-Z]'`
  if [ $? -ne 1 ]
  then echo -e "error: bad prototype(s) in $file"
       echo "$prototypes"
       let "error_counter+=1"
  fi
  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "info: each function prototype must contain low case characters only\n"
     echo -e "info: $file_counter *.h checked. count of errors $error_counter\n"
     exit 1
else echo -e "info: $file_counter *.h checked. count of errors $error_counter\n"
fi
