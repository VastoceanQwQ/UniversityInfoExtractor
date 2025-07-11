#include "pch.h"
#include "WebCrawler.h"
#include <curl/curl.h>
#include <regex>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <direct.h>  
#include <filesystem> 
#include <algorithm>
#ifdef _WIN32
#else
#endif

// 用于 libcurl 下载网页内容时的数据写入
// 把 curl 下载到的数据追加到一个 string 对象中
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}


// 下载网页源码，返回 HTML 的字符串
std::string WebCrawler::fetchHTML() {
    CURL* curl = curl_easy_init();  // 初始化一个 CURL 句柄，用于后续的网络操作。
    std::string readBuffer;         // 用于存储下载到的网页内容。

    if (curl) {
        // 设置 URL
        curl_easy_setopt(curl, CURLOPT_URL, targetURL.c_str()); 
        // 设置浏览器 User-Agent 避免被屏蔽
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
        // 支持重定向
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // 设置写入回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        // 设置写入目标
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // 执行 HTTP 请求，数据通过回调写入 readBuffer
        CURLcode res = curl_easy_perform(curl);  
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        // 释放 CURL 资源
        curl_easy_cleanup(curl); 
    }

    // 写入测试文件 debug.html
    // std::ofstream debug("debug.html");
    // debug << readBuffer;

    return readBuffer;
}

// 解析 HTML，提取高校信息
std::vector<University> WebCrawler::parseUniversities(const std::string& html) {
    std::vector<University> list;

    // 匹配每个高校信息块，提取名称和 ul 列表
    std::regex blockRegex(
        "<dl>[\\s\\S]*?<strong[^>]*title=\\\"([^\\\"]+)\\\"[^>]*>[\\s\\S]*?</strong>[\\s\\S]*?<ul>([\\s\\S]*?)</ul>[\\s\\S]*?</dl>"
    );
    // < dl > [\s\S] * ?
    // 匹配以 <dl> 开头，后面跟着任意内容
    // 
    // < strong[^ > ] * title = \"([^\"]+)\"[^>]*>
    // 匹配 <strong> 标签，要求其属性中有 title = "..."，并用第一个括号捕获 title 属性的内容（即高校名称）
    // 
    // [\s\S] * ? < / strong>
    // 匹配 <strong> 标签内的内容直到 < / strong> 结束
    // 
    // [\s\S] * ? <ul>([\s\S] * ? ) < / ul >
    // 匹配 <ul> 标签及其内容，并用第二个括号捕获 <ul>...< / ul> 之间的内容（即高校的详细信息列表）
    // 
    // [\s\S] * ? < / dl>
    // 匹配剩余内容直到 < / dl> 结束


    // 匹配 ul 列表中的每一项（li）
    std::regex liRegex("<li>(.*?)：([\\s\\S]*?)</li>");
    // <li>
    // 匹配以 <li> 开头的标签
    // 
    // (.* ? )
    // 第一个捕获组，匹配任意字符，直到遇到下一个部分。这里通常是“字段名”，比如“高校所在地”
    // 
    // ：
    // 匹配一个中文全角冒号（注意不是英文冒号），它是字段名和字段值的分隔符
    // 
    // ([\s\S] * ? )
    // 第二个捕获组，非贪婪地匹配任意字符（包括换行），直到遇到 < / li>。这里通常是字段值，比如“北京市”
    // 
    // < / li >
    // 匹配以 < / li> 结尾的标签。


    std::smatch match;                                      // 定义正则匹配结果
    std::string::const_iterator searchStart(html.cbegin()); // 迭代器searchStart，初始化为html字符串的起始位置。

    // 遍历所有高校信息块
    while (std::regex_search(searchStart, html.cend(), match, blockRegex)) {
        std::string name = match[1];
        std::string ul = match[2];
        University u(name); // 先用名称初始化

        std::smatch liMatch;
        std::string::const_iterator liStart = ul.cbegin();

        // 遍历 ul 列表中的每一项，提取关键信息
        while (std::regex_search(liStart, ul.cend(), liMatch, liRegex)) {
            std::string key = liMatch[1];
            std::string value = liMatch[2];

            // 根据 key 设置 University 的属性
            if (key == "高校所在地")    u.setProperty("所在地", value);
            else if (key == "高校类型") u.setProperty("类型", value);
            else if (key == "高校隶属") u.setProperty("隶属", value);
            else if (key == "高校性质") u.setProperty("性质", value);
            else if (key == "学校网址") u.setProperty("网址", value);
            else if (key == "院校特色") {
                // 判断是否包含985/211
                std::string feature = "";
                bool has985 = ul.find(">985<") != std::string::npos;    // 查找985/211子串在ul中首次出现的位置，如果找不到，返回 std::string::npos
                bool has211 = ul.find(">211<") != std::string::npos;    // != std::string::npos 判断查找结果是否有效
                if (has985 && has211) feature = "985/211";
                else if (has985) feature = "985";
                else if (has211) feature = "211";
                u.setProperty("特色", feature);
            }

            liStart = liMatch.suffix().first; // 指向上一次【高校信息】正则匹配结束后的下一个字符
        }

        list.push_back(u);
        searchStart = match.suffix().first;   // 指向上一次【高校信息块】正则匹配结束后的下一个字符
    }

    return list;
}
