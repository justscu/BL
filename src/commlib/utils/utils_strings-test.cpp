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
    assert(o.size() == 6);

    o.clear();
    str = "ab, a c ,, , adf";
    UtilsStr::split(str, ',', o);
    UtilsStr::trim(o, " ");
    assert(o.size() == 5);
    assert(o[0] == "ab");
    assert(o[1] == "a c");
    assert(o[2] == "");
    assert(o[3] == "");
    assert(o[4] == "adf");

    // test is_same_string
    const char *a = "123456789";
    assert( is_same_string(a, "1", 1));
    assert( is_same_string(a, "1234567", 7));
    assert( is_same_string(a, "1233777", 3));
    assert(!is_same_string(a, "1233777", 4));
    assert(!is_same_string(a, "1234777", 6));
    assert(!is_same_string(a, "234567", 6));
    assert( is_same_string(a, "123", 3));
    assert( is_same_string(a, "123456789", 8));
}
