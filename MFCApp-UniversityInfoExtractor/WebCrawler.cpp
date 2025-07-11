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

// ���� libcurl ������ҳ����ʱ������д��
// �� curl ���ص�������׷�ӵ�һ�� string ������
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}


// ������ҳԴ�룬���� HTML ���ַ���
std::string WebCrawler::fetchHTML() {
    CURL* curl = curl_easy_init();  // ��ʼ��һ�� CURL ��������ں��������������
    std::string readBuffer;         // ���ڴ洢���ص�����ҳ���ݡ�

    if (curl) {
        // ���� URL
        curl_easy_setopt(curl, CURLOPT_URL, targetURL.c_str()); 
        // ��������� User-Agent ���ⱻ����
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
        // ֧���ض���
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // ����д��ص�
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        // ����д��Ŀ��
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // ִ�� HTTP ��������ͨ���ص�д�� readBuffer
        CURLcode res = curl_easy_perform(curl);  
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        // �ͷ� CURL ��Դ
        curl_easy_cleanup(curl); 
    }

    // д������ļ� debug.html
    // std::ofstream debug("debug.html");
    // debug << readBuffer;

    return readBuffer;
}

// ���� HTML����ȡ��У��Ϣ
std::vector<University> WebCrawler::parseUniversities(const std::string& html) {
    std::vector<University> list;

    // ƥ��ÿ����У��Ϣ�飬��ȡ���ƺ� ul �б�
    std::regex blockRegex(
        "<dl>[\\s\\S]*?<strong[^>]*title=\\\"([^\\\"]+)\\\"[^>]*>[\\s\\S]*?</strong>[\\s\\S]*?<ul>([\\s\\S]*?)</ul>[\\s\\S]*?</dl>"
    );
    // < dl > [\s\S] * ?
    // ƥ���� <dl> ��ͷ�����������������
    // 
    // < strong[^ > ] * title = \"([^\"]+)\"[^>]*>
    // ƥ�� <strong> ��ǩ��Ҫ������������ title = "..."�����õ�һ�����Ų��� title ���Ե����ݣ�����У���ƣ�
    // 
    // [\s\S] * ? < / strong>
    // ƥ�� <strong> ��ǩ�ڵ�����ֱ�� < / strong> ����
    // 
    // [\s\S] * ? <ul>([\s\S] * ? ) < / ul >
    // ƥ�� <ul> ��ǩ�������ݣ����õڶ������Ų��� <ul>...< / ul> ֮������ݣ�����У����ϸ��Ϣ�б�
    // 
    // [\s\S] * ? < / dl>
    // ƥ��ʣ������ֱ�� < / dl> ����


    // ƥ�� ul �б��е�ÿһ�li��
    std::regex liRegex("<li>(.*?)��([\\s\\S]*?)</li>");
    // <li>
    // ƥ���� <li> ��ͷ�ı�ǩ
    // 
    // (.* ? )
    // ��һ�������飬ƥ�������ַ���ֱ��������һ�����֡�����ͨ���ǡ��ֶ����������硰��У���ڵء�
    // 
    // ��
    // ƥ��һ������ȫ��ð�ţ�ע�ⲻ��Ӣ��ð�ţ��������ֶ������ֶ�ֵ�ķָ���
    // 
    // ([\s\S] * ? )
    // �ڶ��������飬��̰����ƥ�������ַ����������У���ֱ������ < / li>������ͨ�����ֶ�ֵ�����硰�����С�
    // 
    // < / li >
    // ƥ���� < / li> ��β�ı�ǩ��


    std::smatch match;                                      // ��������ƥ����
    std::string::const_iterator searchStart(html.cbegin()); // ������searchStart����ʼ��Ϊhtml�ַ�������ʼλ�á�

    // �������и�У��Ϣ��
    while (std::regex_search(searchStart, html.cend(), match, blockRegex)) {
        std::string name = match[1];
        std::string ul = match[2];
        University u(name); // �������Ƴ�ʼ��

        std::smatch liMatch;
        std::string::const_iterator liStart = ul.cbegin();

        // ���� ul �б��е�ÿһ���ȡ�ؼ���Ϣ
        while (std::regex_search(liStart, ul.cend(), liMatch, liRegex)) {
            std::string key = liMatch[1];
            std::string value = liMatch[2];

            // ���� key ���� University ������
            if (key == "��У���ڵ�")    u.setProperty("���ڵ�", value);
            else if (key == "��У����") u.setProperty("����", value);
            else if (key == "��У����") u.setProperty("����", value);
            else if (key == "��У����") u.setProperty("����", value);
            else if (key == "ѧУ��ַ") u.setProperty("��ַ", value);
            else if (key == "ԺУ��ɫ") {
                // �ж��Ƿ����985/211
                std::string feature = "";
                bool has985 = ul.find(">985<") != std::string::npos;    // ����985/211�Ӵ���ul���״γ��ֵ�λ�ã�����Ҳ��������� std::string::npos
                bool has211 = ul.find(">211<") != std::string::npos;    // != std::string::npos �жϲ��ҽ���Ƿ���Ч
                if (has985 && has211) feature = "985/211";
                else if (has985) feature = "985";
                else if (has211) feature = "211";
                u.setProperty("��ɫ", feature);
            }

            liStart = liMatch.suffix().first; // ָ����һ�Ρ���У��Ϣ������ƥ����������һ���ַ�
        }

        list.push_back(u);
        searchStart = match.suffix().first;   // ָ����һ�Ρ���У��Ϣ�顿����ƥ����������һ���ַ�
    }

    return list;
}
