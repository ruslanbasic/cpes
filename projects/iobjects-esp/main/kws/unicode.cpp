/******************************************************************************
 * @copyright Copyright (c) A1 Company LLC. All rights reserved.
 *****************************************************************************/

#include "unicode.h"
#include <cstring>
#include "miniutf.hpp"

extern "C" bool unicode_stristr(const char* const str1, const char* const str2)
{
  std::string std_str1(str1);
  std::string std_str2(str2);

  return !!strstr(miniutf::lowercase(std_str1).c_str(),
      miniutf::lowercase(std_str2).c_str());
}

/*****************************************************************************/
