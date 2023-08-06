//
// Created by cycastic on 7/25/2023.
//
#include "output.h"

void init_locale(){
#if MS_STDLIB_BUGS
    constexpr char cp_utf16le[] = ".1200";
    setlocale(LC_ALL, cp_utf16le);
    _setmode(_fileno(stdout), _O_WTEXT);
#else
    // The correct locale name may vary by OS, e.g., "en_US.utf8".
    constexpr char locale_name[] = "";
    setlocale( LC_ALL, locale_name );
    std::locale::global(std::locale(locale_name));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
#endif
}