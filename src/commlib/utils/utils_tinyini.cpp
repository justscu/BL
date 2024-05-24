#include <algorithm>
#include <fstream>
#include <string.h>
#include <sstream>
#include "utils_strings.h"
#include "utils_tinyini.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IniRead
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool IniReader::load_ini(const std::string &ini_file) {
    std::ifstream ifs(ini_file, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        snprintf(last_err_, sizeof(last_err_)-1, "open [%s] failed: [%s].", ini_file.c_str(), strerror(errno));
        return false;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        UtilsStr::trim(line, UtilsStr::white_space_delimiters);
        if (line.length() > 2) { parse_line(line); }
    }

    return true;
}

// str为前/后去除“\r\n ”等的字符串
bool IniReader::parse_line(std::string &str) {
    // comment.
    if (str[0] == ';' || str[0] == '#') { return true; }

    // new section
    if (str[0] == '[') {
        return extract_section(str);
    }
    // new key-value
    else {
        return extract_kv(str);
    }
}

// line: [section_name]
bool IniReader::extract_section(std::string &line) {
    UtilsStr::trim(line, "[ ]");

    if (line.empty()) {
        snprintf(last_err_, sizeof(last_err_)-1, "empty line.");
        return false;
    }
    if (!check(line)) {
        return false;
    }

    recent_section_ = line;

    Iter iter = m_.find(recent_section_);
    if (iter != m_.end()) {
        snprintf(last_err_, sizeof(last_err_)-1, "repeat section: %s.", line.c_str());
        return false;
    }

    std::map<Key,Value> m{};
    m_[recent_section_] = m;

    return true;
}

// line: key = value
bool IniReader::extract_kv(std::string &line) {
    // remove comments
    size_t pos = line.find_first_of(";#");
    if (pos != std::string::npos) {
        line.erase(pos);
    }

    Key key;
    Value v;

    pos = line.find_first_of("=");
    if (pos != std::string::npos) {
        key = line.substr(0, pos);
        v   = line.substr(pos+1);
    }

    UtilsStr::trim(key, UtilsStr::white_space_delimiters);
    UtilsStr::trim(v, UtilsStr::white_space_delimiters);

    if (key.empty() || v.empty()) {
        snprintf(last_err_, sizeof(last_err_)-1, "empty key or value: [%s].", line.c_str());
        return false;
    }

    if (!check(key) || !check(v)) { return false; }

    Iter iter = m_.find(recent_section_);
    if (iter == m_.end()) {
        snprintf(last_err_, sizeof(last_err_)-1, "have no section: [%s].", line.c_str());
        return false;
    }

    if (iter->second.find(key) != iter->second.end()) {
        snprintf(last_err_, sizeof(last_err_)-1, "duplicate key: [%s].", line.c_str());
        return false;
    }

    iter->second[key] = v;
    return true;
}

bool IniReader::get_value(const std::string &section,
                          const std::string &key,
                          const char* &value) const {
    CIter it1 = m_.find(section);
    if (it1 == m_.end()) { return false; }

    std::map<Key,Value>::const_iterator it2 = it1->second.find(key);
    if (it2 == it1->second.end()) { return false; }

    value = it2->second.c_str();
    return true;
}

const char* IniReader::operator[](const std::string &section_key) const {
    std::vector<std::string> vec;
    UtilsStr::split(section_key, '.', vec);
    if (vec.size() != 2) {
        snprintf(last_err_, sizeof(last_err_)-1, "IniReader::operator[] error, key[%s]", section_key.c_str());
        return nullptr;
    }

    UtilsStr::trim(vec, UtilsStr::white_space_delimiters);

    const char *ret = nullptr;
    get_value(vec[0], vec[1], ret);
    if (!ret) {
        snprintf(last_err_, sizeof(last_err_)-1, "IniReader::operator[] error, key[%s]", section_key.c_str());
    }

    return ret;
}

void IniReader::print(std::string &o) const {
    for (CIter it1 = m_.begin(); it1 != m_.end(); ++it1) {
        o.append("[").append(it1->first).append("]\n");
        for (std::map<Key,Value>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            o.append("    ").append(it2->first).append(" : ").append(it2->second).append("\n");
        }
    }
}

bool IniReader::check(const std::string &str) {
    bool ok = true;
    const std::string con(".,-_:/\\ ");

    size_t len = str.length();
    for (size_t i = 0; i < len; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') { continue; }
        if (str[i] >= 'a' && str[i] <= 'z') { continue; }
        if (str[i] >= '0' && str[i] <= '9') { continue; }


        if (str[i] == '.') { continue; }
        if (str[i] == '-') { continue; }
        if (str[i] == '_') { continue; }

        if (con.find(str[i]) != std::string::npos) { continue; }
        ok = false;
    }

    if (!ok) {
        snprintf(last_err_, sizeof(last_err_)-1, "error string[%s], only support(A-Z a-z 0-9 %s)", str.c_str(), con.c_str());
    }
    return ok;
}
