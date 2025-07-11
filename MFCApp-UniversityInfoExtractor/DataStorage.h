#pragma once
#include "University.h"
#include <vector>
#include <string>

// DataStorage类 高校信息的存储和加载
class DataStorage {
private:
    std::string dataFile = "universities.csv";

public:
    void saveData(const std::vector<University>& data);
    std::vector<University> loadData();
};
