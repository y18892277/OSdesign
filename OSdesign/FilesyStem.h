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
    FileSystemNode* root;            // ��Ŀ¼
    FileSystemNode* current;         // ����Ŀ¼
    FileSystemNode* home;            // ��Ŀ¼
    std::unordered_map<std::string, User> users; // �����û�

public:
    User* currentUser = nullptr;     // ��ǰ��¼���û�

    FileSystem();                     // ���캯��
    ~FileSystem();                    // ��������

    // ����·�����ҽڵ�
    FileSystemNode* getNodeByPath(const std::string& path);

    // ����ͽ����ļ�����
    static std::string encodeContent(const std::string& content);
    static std::string decodeContent(const std::string& encodedContent);

    void saveNode(std::ofstream& out, FileSystemNode* node, const std::string& prefix);

    // ����ͼ����ļ�ϵͳ
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

    // �����û���Ϣ
    void saveUsersToFile(const std::string& filename);
    void loadUsersFromFile(const std::string& filename);

    // ���úͻ�ȡȨ��
    void setPermissions(const std::string& path, const std::string& newPermissions);
    std::string getPermissions(const std::string& path);

    // �û�����
    void addUser(const std::string& username, const std::string& password, const std::string& group);
    void changeUserPassword(const std::string& username, const std::string& newPassword);
    bool login(const std::string& username, const std::string& password);

    // Ȩ�޼��
    bool checkPermissions(FileSystemNode* node, char permissionType);

    // �������ļ���Ŀ¼
    void create(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group);

    // ɾ���ļ���Ŀ¼
    void remove(const std::string& path);

    // ���ĵ�ǰ����Ŀ¼
    void changeDirectory(const std::string& path);

    // �г���ǰĿ¼������
    void listDirectory(bool longFormat = false);

    // ��ȡ�ļ�����
    void readFile(const std::string& path);

    // ��ȡ��ǰ����Ŀ¼�ľ���·��
    std::string getCurrentDirectoryPath();

    void load(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group);
};

