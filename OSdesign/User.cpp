#include "User.h"

// 构造函数实现
User::User(const std::string& username, const std::string& password,
    const std::string& group, const std::string& homeDirectory)
    : username(username),
    password(password),
    group(group),
    homeDirectory(homeDirectory) {
}
