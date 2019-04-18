#!/bin/bash

# copyright header and source files ###########################################

header="$(cat <<-EOF
/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/
EOF
)"

echo "info: checking copyright header..."

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
  if [ "$(head -n 3 $file)" != "$header" ]
  then echo -e "error: incorrect header in the file $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "\ninfo: first 3 lines of each header and source file must contain the next header:\n\n$header\n"
     echo -e "info: $file_counter *.h, *.c checked. count of errors $error_counter\n"
     exit 1
fi

# makefiles ###################################################################

header="$(cat <<-EOF
###############################################################################
# @copyright Copyright (c) A1 Company LLC. All rights reserved.
###############################################################################
EOF
)"

exclude="^\."`
       `"(/build"`
       `"|/eclipse"`
       `"|/components/esp-adf"`
       `"|/components/libfvad"`
       `"|/components/kws_guess"`
       `"|/components/libwebsockets"`
       `"|/components/mbedtls"`
       `"|/components/miniutf"`
       `"|/components/unity"`
       `"|/components/esp_http_client"`
       `"|/components/zlib)"

files=$(find . -type f \( -name "component.mk" -o -name "sdkconfig.defaults" \) | egrep -v $exclude)

for file in $files
do
  if [ "$(head -n 3 $file)" != "$header" ]
  then echo -e "error: incorrect header in the file $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then echo -e "\ninfo: first 3 lines of each makefile must contain the next header:\n\n$header\n"
     echo -e "info: $file_counter checked. count of errors $error_counter\n"
     exit 1
else echo -e "info: $file_counter checked. count of errors $error_counter\n"
fi
