//
// From https://stackoverflow.com/questions/22950412/c-cant-get-wcout-to-print-unicode-and-leave-cout-working
//

#ifndef NEXUS_OUTPUT_H
#define NEXUS_OUTPUT_H

#include <iostream>
#include <locale>
#include <clocale>

#ifndef MS_STDLIB_BUGS
#  if (_MSC_VER || __MINGW32__ || __MSVCRT__)
#    define MS_STDLIB_BUGS 1
#  else
#    define MS_STDLIB_BUGS 0
#  endif
#endif

#if MS_STDLIB_BUGS

#  include <io.h>
#  include <fcntl.h>

#endif

void init_locale();

#endif //NEXUS_OUTPUT_H
