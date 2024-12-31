#include "FileSystemNode.h"

// 构造函数实现
FileSystemNode::FileSystemNode(std::string name, bool isDirectory, std::string owner,
    std::string group,
    std::string userPermissions,
    std::string groupPermissions,
    std::string otherPermissions,
    FileSystemNode* parent)
    : name(name),
    isDirectory(isDirectory),
    owner(owner),
    group(group),
    userPermissions(userPermissions),
    groupPermissions(groupPermissions),
    otherPermissions(otherPermissions),
    parent(parent),
    isBeingEdited(false) {
}

// 析构函数实现，释放子节点内存
FileSystemNode::~FileSystemNode() {
    for (auto& child : children) {
        delete child.second;
    }
}

// 设置权限实现
void FileSystemNode::setPermissions(const std::string& newPermissions) {
    if (newPermissions.size() == 9) {
        userPermissions = newPermissions.substr(0, 3);
        groupPermissions = newPermissions.substr(3, 3);
        otherPermissions = newPermissions.substr(6, 3);
    }
}

// 获取权限实现
std::string FileSystemNode::getPermissions() const {
    return userPermissions + groupPermissions + otherPermissions;
}

// 格式化文件或目录的权限表示实现
std::string FileSystemNode::formatPermissions() const {
    std::ostringstream oss;
    oss << (isDirectory ? 'd' : '-') << userPermissions << groupPermissions
        << otherPermissions << " " << owner << " " << group;
    return oss.str();
}
