#include "User.h"

// ���캯��ʵ��
User::User(const std::string& username, const std::string& password,
    const std::string& group, const std::string& homeDirectory)
    : username(username),
    password(password),
    group(group),
    homeDirectory(homeDirectory) {
}
