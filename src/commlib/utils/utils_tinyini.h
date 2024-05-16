#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <map>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 间单的ini格式配置文件
//
// 支持的注释: '#'、';'
//
// 包含 `Section`和`key-value`两种数据
//
// `Section`: 以`[ ]`包含；必须含有Section. 如“[section]”
//
// `key-value`: 如“key1=value”, “key2=value2;”
//
// [SHL1B]
// key1 =.e value#;
// key2 = value; #
// key3 = S #value
//
// [SHL2]
// key1 = x,x,v,y,w
//
// [SZ]
// uri1=10.10.25.26:9988
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class IniReader {
using Section=std::string;
using Key=std::string;
using Value=std::string;
using Iter  = std::map<Section, std::map<Key,Value>>::iterator;
using CIter = std::map<Section, std::map<Key,Value>>::const_iterator;

public:
    bool load_ini(const std::string &ini_file);

    bool get_value(const std::string &section, const std::string &key, const char* &value) const;
    // Section.Key
    const char* operator[](const std::string &section_key);

    // 将结果输出到o中
    void print(std::string &o) const;

    const char* last_err() const { return last_err_; }

private:
    bool parse_line(std::string &line);
    bool extract_section(std::string &line);
    bool extract_kv(std::string &line);

    bool check(const std::string &str);

private:
    char last_err_[256] = {0};

    std::map<Section, std::map<Key,Value>> m_;
    Section                   recent_section_;
};
