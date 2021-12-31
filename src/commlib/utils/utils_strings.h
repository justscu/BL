#pragma once

#include <vector>
#include <string>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// string utils.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class UtilsStr {
public:
    static void split(const std::string &str, const char delim, std::vector<std::string> &out);
    static void trim(std::string &str, const std::string &delim);
    // "ab, a c ,, , adf"   =>   "ab,a c,,,adf"
    static void trim(std::vector<std::string> &vec, const std::string &delim);
    // "ab, a c ,, , adf"   =>   "ab,a c,adf"
    static void trim_and_rm(std::vector<std::string> &vec, const std::string &delim);
    static void to_lower(std::string &str);
    static void to_upper(std::string &str);
    static void replace(std::string &str, const std::string &from, const std::string &to);

public:
    static const char *white_space_delimiters;
};
