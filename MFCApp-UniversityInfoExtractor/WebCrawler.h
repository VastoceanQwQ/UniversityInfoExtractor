#pragma once
#include <vector>
#include "University.h"

// WebCrawler类 抓取和解析高校信息网页
class WebCrawler {
private:
    std::string targetURL; // 目标网页的 URL

public:
    // 构造函数，指定目标 URL
    WebCrawler(const std::string& url) : targetURL(url) {}

    // 下载网页源码，返回 HTML 字符串
    std::string fetchHTML();

    // 解析 HTML，提取高校信息，返回 University 对象列表
    std::vector<University> parseUniversities(const std::string& html);
};
