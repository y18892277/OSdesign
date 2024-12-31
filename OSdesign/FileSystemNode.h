#pragma once
#include <string>
#include <map>
#include <sstream>

class FileSystemNode {
public:
    std::string name;                // 文件或目录名, 不包含路径
    bool isDirectory;                // 是否为目录
    std::string owner;               // 拥有者
    std::string group;               // 用户组
    std::string content;             // 文件内容
    std::map<std::string, FileSystemNode*> children;  // 目录内容
    std::string userPermissions;     // 用户权限
    std::string groupPermissions;    // 用户组权限
    std::string otherPermissions;    // 其他用户权限
    FileSystemNode* parent;          // 指向父节点的指针
    bool isBeingEdited;              // 编辑状态

    // 构造函数
    FileSystemNode(std::string name, bool isDirectory, std::string owner,
        std::string group,
        std::string userPermissions = "rwx",  // 默认用户权限
        std::string groupPermissions = "r-x",  // 默认用户组权限
        std::string otherPermissions = "r-x",  // 默认其他用户权限
        FileSystemNode* parent = nullptr);

    // 析构函数
    ~FileSystemNode();

    // 设置权限
    void setPermissions(const std::string& newPermissions);

    // 获取权限
    std::string getPermissions() const;

    // 格式化文件或目录的权限表示
    std::string formatPermissions() const;
};

