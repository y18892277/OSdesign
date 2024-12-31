#include "FileSystemNode.h"

// ���캯��ʵ��
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

// ��������ʵ�֣��ͷ��ӽڵ��ڴ�
FileSystemNode::~FileSystemNode() {
    for (auto& child : children) {
        delete child.second;
    }
}

// ����Ȩ��ʵ��
void FileSystemNode::setPermissions(const std::string& newPermissions) {
    if (newPermissions.size() == 9) {
        userPermissions = newPermissions.substr(0, 3);
        groupPermissions = newPermissions.substr(3, 3);
        otherPermissions = newPermissions.substr(6, 3);
    }
}

// ��ȡȨ��ʵ��
std::string FileSystemNode::getPermissions() const {
    return userPermissions + groupPermissions + otherPermissions;
}

// ��ʽ���ļ���Ŀ¼��Ȩ�ޱ�ʾʵ��
std::string FileSystemNode::formatPermissions() const {
    std::ostringstream oss;
    oss << (isDirectory ? 'd' : '-') << userPermissions << groupPermissions
        << otherPermissions << " " << owner << " " << group;
    return oss.str();
}
