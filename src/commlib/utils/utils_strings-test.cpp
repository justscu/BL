#include <assert.h>
#include "utils_strings.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// string utils test
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void StrUtils_test() {
    std::string str("  abcde   ");
    UtilsStr::trim(str, UtilsStr::white_space_delimiters);
    assert(str == std::string("abcde"));

    str = "\t\tabcde ";
    UtilsStr::trim(str, UtilsStr::white_space_delimiters);
    assert(str == std::string("abcde"));

    str = " abcde\r\n ";
    UtilsStr::trim(str, UtilsStr::white_space_delimiters);
    assert(str == std::string("abcde"));


    str = "ABcde\t";
    UtilsStr::to_lower(str);
    assert(str == std::string("abcde\t"));

    str = "ABcde\t";
    UtilsStr::to_upper(str);
    assert(str == std::string("ABCDE\t"));

    str = "Abcdcec\t";
    UtilsStr::replace(str, "c", "CCC");
    assert(str == std::string("AbCCCdCCCeCCC\t"));

    str = "AbcAbcABCabc";
    UtilsStr::replace(str, "bc", "F");
    assert(str == std::string("AFAFABCaF"));

    str = "ab,ac,ef , aaa,,a,";
    std::vector<std::string> o;
    UtilsStr::split(str, ',', o);
    assert(o.size() == 5);
}
