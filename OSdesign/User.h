#pragma once
#include <string>
class User {
public:
    std::string username;       // �û���
    std::string password;       // ����
    std::string group;          // �û���
    std::string homeDirectory;  // ��Ŀ¼

    // ���캯��
    User(const std::string& username, const std::string& password,
        const std::string& group, const std::string& homeDirectory);
};

