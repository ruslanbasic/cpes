#!/bin/bash

# astyle ######################################################################

echo "info: checking astyle `astyle --version | awk '{print $(NF)}'` in header and source files..."

exclude="^\."`
       `"(/build"`
       `"|/eclipse"`
       `"|/components/esp-adf"`
       `"|/components/edbg"`
       `"|/components/bme280"`
       `"|/components/kws_guess"`
       `"|/components/curl"`
       `"|/components/libfvad"`
       `"|/components/libwebsockets"`
       `"|/components/mbedtls"`
       `"|/components/miniutf"`
       `"|/components/stpm3x"`
       `"|/components/unity"`
       `"|/components/esp_http_client"`
       `"|/components/zlib)"


if [[ "$@" == "--astyle-all" ]]
then
  files=$(find . -type f \( -name "*.h" -o -name "*.c" \) | egrep -v $exclude)
else
  # list of files to be pushed
  branch_to_compare="origin/`git branch | grep \* | cut -d ' ' -f2`"
  if ! git branch -r | grep -q "$branch_to_compare"; then
    # branch hasn't been created remotely, taking parent branch
    branch_to_compare=`git show-branch | grep '*' | grep -v "$(git rev-parse --abbrev-ref HEAD)" | head -n1 | sed 's/.*\[\(.*\)\].*/\1/' | sed 's/[\^~].*//'`
  fi
  files=$(git diff --stat --cached --diff-filter=d --name-only "$branch_to_compare" | sed 's|^|./|' | egrep '\.h$|\.c$' | egrep -v $exclude)
fi

file_counter=0
error_counter=0

for file in $files
do
  astyle_result=$(astyle --suffix=none --style=allman --indent=spaces=2 --indent-switches $file)
  if [ "$astyle_result" != "Unchanged  $file" ]
  then echo -e "error: astyle dont like $file"
       let "error_counter+=1"
  fi

  let "file_counter+=1"
done

if [ $error_counter -ne 0 ]
then
     echo -e "info: $file_counter checked. count of errors $error_counter\n"
     exit 1
else echo -e "info: $file_counter checked. count of errors $error_counter\n"
fi
