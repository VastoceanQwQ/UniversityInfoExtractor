#pragma once
#include <string>
#include <map>

// University�� ��У��Ϣ�Ļ�����Ԫ
class University {
private:
    std::string name;                              // ѧУ����
    std::map<std::string, std::string> properties; // ���ԣ����ڵء����͡����ʡ���ɫ����ַ��������

public:
    // ���캯������ʼ������
    University(std::string name)
        : name(name){
    }
    std::string getName() const { return name; }        // ��ȡѧУ����
    
    // ��������
    void setProperty(const std::string& key, const std::string& value) {
        properties[key] = value;
    }

    // ��ȡ����ֵ�����������򷵻ؿ��ַ�����
    std::string getProperty(const std::string& key) const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : "";
    }

};
