#!/bin/bash

# footer in header and source files ###########################################

footer="$(cat <<-EOF

/*****************************************************************************/
EOF
)"

echo "info: checking footers..."

exclude="^\."`
       `"(/build"`
       `"|/eclipse"`
       `"|/components/esp-adf"`
       `"|/components/edbg"`
       `"|/components/bme280"`
       `"|/components/curl"`
       `"|/components/kws_guess"`
       `"|/components/libfvad"`
       `"|/components/libwebsockets"`
       `"|/components/mbedtls"`
       `"|/components/miniutf"`
       `"|/components/stpm3x"`
       `"|/components/unity"`
       `"|/components/esp_http_client"`
       `"|/components/zlib)"

files=$(find . -type f \( -name "*.h" -o -name "*.c" \) | egrep -v $exclude)

file_counter=0
error_counter=0

for file in $files
do
  if [ "$(tail -n 2 $file)" != "$footer" ]
  then echo -e "error: incorrect footer in the file $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "\ninfo: last 2 lines of each header and source file must contain the next footer:\n$footer\n"
     echo -e "info: $file_counter *.h, *.c checked. count of errors $error_counter\n"
     exit 1
fi

# footer in makefiles #########################################################

footer="$(cat <<-EOF

###############################################################################
EOF
)"

exclude="^\."`
       `"(/build"`
       `"|/eclipse"`
       `"|/components/esp-adf"`
       `"|/components/kws_guess"`
       `"|/components/libfvad"`
       `"|/components/libwebsockets"`
       `"|/components/mbedtls"`
       `"|/components/miniutf"`
       `"|/components/unity"`
       `"|/components/esp_http_client"`
       `"|/components/zlib)"

files=$(find . -type f \( -name "component.mk" \) | egrep -v $exclude)

for file in $files
do
  if [ "$(tail -n 2 $file)" != "$footer" ]
  then echo -e "error: incorrect footer in the file $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "\ninfo: last 2 lines of each makefile must contain the next footer:\n$footer\n"
     echo -e "info: $file_counter checked. count of errors $error_counter\n"
     exit 1
else echo -e "info: $file_counter checked. count of errors $error_counter\n"
fi
