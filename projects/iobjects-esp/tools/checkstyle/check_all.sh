#!/bin/bash

set -e
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
cd "$dir/../../"

# filenames lower case ########################################################
$dir/check_file_name.sh

# folders lower case ##########################################################
$dir/check_folder_name.sh

# prototypes in headers are lower case ########################################
$dir/check_prototypes.sh

# copyright ###################################################################
$dir/check_copyright.sh

# footer ######################################################################
$dir/check_footer.sh

# astyle formatting ###########################################################
$dir/check_astyle.sh "$@"

###############################################################################

echo "info: OK"
exit 0