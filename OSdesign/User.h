#pragma once
#include <string>
class User {
public:
    std::string username;       // 用户名
    std::string password;       // 密码
    std::string group;          // 用户组
    std::string homeDirectory;  // 家目录

    // 构造函数
    User(const std::string& username, const std::string& password,
        const std::string& group, const std::string& homeDirectory);
};

