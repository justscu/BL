#include <algorithm>
#include <fstream>
#include <string.h>
#include <sstream>
#include "tinyini.h"
#include "utils.h"
#include "log.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// IniRead
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool IniReader::load_ini(const std::string &ini_file) {
    std::ifstream ifs(ini_file, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        log_err("open [%s] failed: err[%s]", ini_file.c_str(), strerror(errno));
        return false;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        UtilsStr::trim(line, UtilsStr::white_space_delimiters);
        if (line.length() > 2) { parse_line(line); }
    }

    log_info("read [%s] success", ini_file.c_str());

    print();

    return true;
}

// str为前/后去除“\r\n ”等的字符串
void IniReader::parse_line(std::string &str) {
    // comment.
    if (str[0] == ';' || str[0] == '#') { return; }

    // new section
    if (str[0] == '[') {
        extract_section(str);
    }
    // new key-value
    else {
        extract_kv(str);
    }
}

// line: [section_name]
void IniReader::extract_section(std::string &line) {
    UtilsStr::trim(line, "[ ]");

    if (line.empty() || !check(line)) { return; }

    recent_section_ = line;

    Iter iter = m_.find(recent_section_);
    if (iter != m_.end()) {
        critical_error("repeat section: " + line);
        exit(-1);
    }

    std::map<Key,Value> m{};
    m_[recent_section_] = m;
}

// line: key = value
void IniReader::extract_kv(std::string &line) {
    const size_t len = line.length();

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
        critical_error(line);
        exit(-1);
    }

    if (!check(key) || !check(v)) { return; }

    Iter iter = m_.find(recent_section_);
    if (iter == m_.end()) {
        critical_error("have no section:" + line);
        exit(-1);
    }

    if (iter->second.find(key) != iter->second.end()) {
        critical_error("duplicate key: " + line);
        exit(-1);
    }

    iter->second[key] = v;
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
    if (vec.size() != 2) { return nullptr; }

    UtilsStr::trim(vec, UtilsStr::white_space_delimiters);

    const char *ret = nullptr;
    get_value(vec[0], vec[1], ret);
    return ret;
}

void IniReader::print() const {
    for (CIter it1 = m_.begin(); it1 != m_.end(); ++it1) {
        log_info("[%s]", it1->first.c_str());
        for (std::map<Key,Value>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            log_info("    %s : %s", it2->first.c_str(), it2->second.c_str());
        }
    }
}

bool IniReader::check(const std::string &str) const {
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
        critical_error(str + ": string error: only support (A-Z a-z 0-9 " + con + ")");
        exit(-1);
    }
    return ok;
}

void IniReader::critical_error(const std::string &err_str) const {
    fprintf(stdout, "critical_error: %s \n", err_str.c_str());
    exit(-1);
}
