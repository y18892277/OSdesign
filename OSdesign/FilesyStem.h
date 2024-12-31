#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "FileSystemNode.h"
#include "User.h"

class FileSystem {
private:
    FileSystemNode* root;            // 根目录
    FileSystemNode* current;         // 工作目录
    FileSystemNode* home;            // 家目录
    std::unordered_map<std::string, User> users; // 所有用户

public:
    User* currentUser = nullptr;     // 当前登录的用户

    FileSystem();                     // 构造函数
    ~FileSystem();                    // 析构函数

    // 根据路径查找节点
    FileSystemNode* getNodeByPath(const std::string& path);

    // 编码和解码文件内容
    static std::string encodeContent(const std::string& content);
    static std::string decodeContent(const std::string& encodedContent);

    void saveNode(std::ofstream& out, FileSystemNode* node, const std::string& prefix);

    // 保存和加载文件系统
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

    // 保存用户信息
    void saveUsersToFile(const std::string& filename);
    void loadUsersFromFile(const std::string& filename);

    // 设置和获取权限
    void setPermissions(const std::string& path, const std::string& newPermissions);
    std::string getPermissions(const std::string& path);

    // 用户管理
    void addUser(const std::string& username, const std::string& password, const std::string& group);
    void changeUserPassword(const std::string& username, const std::string& newPassword);
    bool login(const std::string& username, const std::string& password);

    // 权限检查
    bool checkPermissions(FileSystemNode* node, char permissionType);

    // 创建新文件或目录
    void create(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group);

    // 删除文件或目录
    void remove(const std::string& path);

    // 更改当前工作目录
    void changeDirectory(const std::string& path);

    // 列出当前目录的内容
    void listDirectory(bool longFormat = false);

    // 读取文件内容
    void readFile(const std::string& path);

    // 获取当前工作目录的绝对路径
    std::string getCurrentDirectoryPath();

    void load(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group);
};

