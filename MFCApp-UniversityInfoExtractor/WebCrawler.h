#pragma once
#include <vector>
#include "University.h"

// WebCrawler�� ץȡ�ͽ�����У��Ϣ��ҳ
class WebCrawler {
private:
    std::string targetURL; // Ŀ����ҳ�� URL

public:
    // ���캯����ָ��Ŀ�� URL
    WebCrawler(const std::string& url) : targetURL(url) {}

    // ������ҳԴ�룬���� HTML �ַ���
    std::string fetchHTML();

    // ���� HTML����ȡ��У��Ϣ������ University �����б�
    std::vector<University> parseUniversities(const std::string& html);
};
