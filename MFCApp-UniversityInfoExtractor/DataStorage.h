#pragma once
#include "University.h"
#include <vector>
#include <string>

// DataStorage�� ��У��Ϣ�Ĵ洢�ͼ���
class DataStorage {
private:
    std::string dataFile = "universities.csv";

public:
    void saveData(const std::vector<University>& data);
    std::vector<University> loadData();
};
