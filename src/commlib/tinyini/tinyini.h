#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <map>

class IniReader {
using Section=std::string;
using Key=std::string;
using Value=std::string;
using Iter  = std::map<Section, std::map<Key,Value>>::iterator;
using CIter = std::map<Section, std::map<Key,Value>>::const_iterator;

public:
    bool load_ini(const std::string &ini_file);
    void parse_line(std::string &line);
    void extract_section(std::string &line);
    void extract_kv(std::string &line);

    bool get_value(const std::string &section, const std::string &key, const char* &value) const;

    void print() const;

private:
    bool check(const std::string &str) const;
    void critical_error(const std::string &err_str) const;

private:
    std::map<Section, std::map<Key,Value>> m_;
    Section                   recent_section_;
};
