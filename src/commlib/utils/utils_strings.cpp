#include <string.h>
#include <algorithm>
#include <sstream>
#include "utils_strings.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// string utils.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char* UtilsStr::white_space_delimiters = " \t\n\r";

void UtilsStr::split(const std::string &str, const char delim, std::vector<std::string> &out) {
    std::istringstream ifs(str);

    std::string line;
    while (std::getline(ifs, line, delim)) {
        out.push_back(line);
    }
}

void UtilsStr::trim(std::string &str, const std::string &delim) {
    str.erase(str.find_last_not_of(delim) + 1);
    str.erase(0, str.find_first_not_of(delim));
}

void UtilsStr::trim(std::vector<std::string> &vec, const std::string &delim) {
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
        trim(*it, delim);
    }
}

void UtilsStr::trim_and_rm(std::vector<std::string> &vec, const std::string &delim) {
    std::vector<std::string>::iterator it = vec.begin();
    while (it != vec.end()) {
        trim(*it, delim);
        if (it->empty()) {
            it = vec.erase(it);
        }
        else {
            ++it;
        }
    }
}

void UtilsStr::to_lower(std::string &str) {
    std::for_each(str.begin(), str.end(), [](char &c) { c = std::tolower(c); });
}

void UtilsStr::to_upper(std::string &str) {
    std::for_each(str.begin(), str.end(), [](char &c) { c = std::toupper(c); });
}

void UtilsStr::replace(std::string &str, const std::string &from, const std::string &to) {
    if (str.empty()) return;

    std::size_t i = 0;
    while ((i = str.find(from, i)) != std::string::npos) {
        str.replace(i, from.size(), to);
        i += to.size();
    }
}

// vec: ["abc", "123", "12"]
// out: "abc,123,12"
std::string UtilsStr::tostr(const std::vector<uint16_t> &vec) {
    std::string o;
    o.clear();

    char buf[64] = {0};

    auto it = vec.begin();
    if (it != vec.end()) {
        snprintf(buf, sizeof(buf)-1, "%u", *it);
        o.append(buf);

        for (++it; it != vec.end(); ++it) {
            snprintf(buf, sizeof(buf)-1, ",%u", *it);
            o.append(buf);
        }
    }


    return o;
}
