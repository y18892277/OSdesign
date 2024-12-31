#pragma once
#include <string>
#include <map>
#include <sstream>

class FileSystemNode {
public:
    std::string name;                // �ļ���Ŀ¼��, ������·��
    bool isDirectory;                // �Ƿ�ΪĿ¼
    std::string owner;               // ӵ����
    std::string group;               // �û���
    std::string content;             // �ļ�����
    std::map<std::string, FileSystemNode*> children;  // Ŀ¼����
    std::string userPermissions;     // �û�Ȩ��
    std::string groupPermissions;    // �û���Ȩ��
    std::string otherPermissions;    // �����û�Ȩ��
    FileSystemNode* parent;          // ָ�򸸽ڵ��ָ��
    bool isBeingEdited;              // �༭״̬

    // ���캯��
    FileSystemNode(std::string name, bool isDirectory, std::string owner,
        std::string group,
        std::string userPermissions = "rwx",  // Ĭ���û�Ȩ��
        std::string groupPermissions = "r-x",  // Ĭ���û���Ȩ��
        std::string otherPermissions = "r-x",  // Ĭ�������û�Ȩ��
        FileSystemNode* parent = nullptr);

    // ��������
    ~FileSystemNode();

    // ����Ȩ��
    void setPermissions(const std::string& newPermissions);

    // ��ȡȨ��
    std::string getPermissions() const;

    // ��ʽ���ļ���Ŀ¼��Ȩ�ޱ�ʾ
    std::string formatPermissions() const;
};

