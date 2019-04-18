#!/bin/bash

find . \
  -name '*.c' \
  -o -name '*.h' \
  -o -name '*.mk' \
  -o -name makefile \
  -o -name '*.sh' \
  -o -name '*.md' \
  -o -name '*.csv' \
  -o -name '*.cpp' \
  -o -name '*.mk.bak' \
  | grep -v rebranding.sh \
  | xargs sed -i 's/@copyright Copyright (c) A1 Company. All rights reserved./@copyright Copyright (c) A1 Company LLC. All rights reserved./g'


find . -name '*.md' \
  | xargs sed -i 's/Copyright (c) A1 Company/Copyright (c) A1 Company LLC/g'