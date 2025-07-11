#include "pch.h"
#include "DataStorage.h"
#include <fstream>
#include <iostream>
#include <sstream>

// �洢���ݵ�ʵ��
void DataStorage::saveData(const std::vector<University>& data) {
	std::ofstream ofs(dataFile); // ���ļ�
    if (!ofs) {
        std::cerr << "�޷����ļ�����д�롣\n";
        return;
    }

    // ��ͷ
    ofs << "����,���ڵ�,����,����,����,��ɫ,ѧУ��ַ\n";

	// д��ÿ����ѧ����Ϣ
    for (const auto& u : data) {
        ofs << u.getName() << ","
            << u.getProperty("���ڵ�") << ","
            << u.getProperty("����") << ","
            << u.getProperty("����") << ","
            << u.getProperty("����") << ","
            << u.getProperty("��ɫ") << ","
            << u.getProperty("��ַ") << "\n";
    }
    ofs.close();
}

// �������ݵ�ʵ�֣�����vector��ŵ��ڴ���
std::vector<University> DataStorage::loadData() {
    std::ifstream ifs(dataFile);
    std::vector<University> list;
    if (!ifs) return list;

    std::string line;
    getline(ifs, line);  // ������ͷ

	// ���ж�ȡ����
    while (getline(ifs, line)) {
        std::stringstream ss(line);
        std::string  name, loc, type, nature, belong, feature, url;

        // �ָ��ַ���
        getline(ss, name, ',');
        getline(ss, loc, ',');
        getline(ss, type, ',');
        getline(ss, nature, ',');
        getline(ss, belong, ',');
        getline(ss, feature, ',');
        getline(ss, url, ',');

        // �ֱ�д����Ϣ
        University u(name);
        u.setProperty("���ڵ�", loc);
        u.setProperty("����", type);
        u.setProperty("����", nature);
        u.setProperty("����", belong);
        u.setProperty("��ɫ", feature);
        u.setProperty("��ַ", url);

		// ��ӵ��б��β
        list.push_back(u);
    }

    return list;
}

