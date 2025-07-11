#include "pch.h"
#include "DataStorage.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 存储数据的实现
void DataStorage::saveData(const std::vector<University>& data) {
	std::ofstream ofs(dataFile); // 打开文件
    if (!ofs) {
        std::cerr << "无法打开文件进行写入。\n";
        return;
    }

    // 表头
    ofs << "名称,所在地,类型,性质,隶属,特色,学校网址\n";

	// 写入每个大学的信息
    for (const auto& u : data) {
        ofs << u.getName() << ","
            << u.getProperty("所在地") << ","
            << u.getProperty("类型") << ","
            << u.getProperty("性质") << ","
            << u.getProperty("隶属") << ","
            << u.getProperty("特色") << ","
            << u.getProperty("网址") << "\n";
    }
    ofs.close();
}

// 加载数据的实现，利用vector存放到内存中
std::vector<University> DataStorage::loadData() {
    std::ifstream ifs(dataFile);
    std::vector<University> list;
    if (!ifs) return list;

    std::string line;
    getline(ifs, line);  // 跳过表头

	// 逐行读取数据
    while (getline(ifs, line)) {
        std::stringstream ss(line);
        std::string  name, loc, type, nature, belong, feature, url;

        // 分割字符串
        getline(ss, name, ',');
        getline(ss, loc, ',');
        getline(ss, type, ',');
        getline(ss, nature, ',');
        getline(ss, belong, ',');
        getline(ss, feature, ',');
        getline(ss, url, ',');

        // 分别写入信息
        University u(name);
        u.setProperty("所在地", loc);
        u.setProperty("类型", type);
        u.setProperty("性质", nature);
        u.setProperty("隶属", belong);
        u.setProperty("特色", feature);
        u.setProperty("网址", url);

		// 添加到列表队尾
        list.push_back(u);
    }

    return list;
}

