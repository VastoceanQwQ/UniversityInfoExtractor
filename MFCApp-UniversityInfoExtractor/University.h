#pragma once
#include <string>
#include <map>

// University类 高校信息的基本单元
class University {
private:
    std::string name;                              // 学校名称
    std::map<std::string, std::string> properties; // 属性（所在地、类型、性质、特色、网址、隶属）

public:
    // 构造函数，初始化名称
    University(std::string name)
        : name(name){
    }
    std::string getName() const { return name; }        // 获取学校名称
    
    // 设置属性
    void setProperty(const std::string& key, const std::string& value) {
        properties[key] = value;
    }

    // 获取属性值（若不存在则返回空字符串）
    std::string getProperty(const std::string& key) const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : "";
    }

};
