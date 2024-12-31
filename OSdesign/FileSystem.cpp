#include "FileSystem.h"
#include <iomanip>

// ���캯��ʵ��
FileSystem::FileSystem() {
    root = new FileSystemNode("/", true, "root", "root");
    current = root;

    home = new FileSystemNode("root", true, "root", "root", "rwx", "r-x", "r-x", root);
    root->children["root"] = home;

    users.emplace(std::piecewise_construct,
        std::forward_as_tuple("root"),
        std::forward_as_tuple("root", "password", "root", "/root"));
}

// ��������ʵ��
FileSystem::~FileSystem() {
    delete root;
}

// ����·�����ҽڵ�ʵ��
FileSystemNode* FileSystem::getNodeByPath(const std::string& path) {
    if (path.empty()) return nullptr;

    std::string absolutePath = path;
    // ��������·��������ת��Ϊ����·��
    if (path[0] != '/') {
        absolutePath = getCurrentDirectoryPath() + path;
    }

    // ����ԭ�еľ���·�������߼�
    std::vector<std::string> parts;
    std::istringstream iss(absolutePath);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    FileSystemNode* node = root;
    for (const auto& p : parts) {
        if (node->children.find(p) == node->children.end()) return nullptr;
        node = node->children[p];
    }
    return node;
}

// �����ļ�����ʵ��
std::string FileSystem::encodeContent(const std::string& content) {
    std::string encoded;
    for (char c : content) {
        if (c == '\n') {
            encoded += "\\n"; // ���з��滻
        }
        else if (c == '\\') {
            encoded += "\\\\"; // ��б���滻
        }
        else {
            encoded += c;
        }
    }
    return encoded;
}

// �����ļ�����ʵ��
std::string FileSystem::decodeContent(const std::string& encodedContent) {
    std::string content;
    for (size_t i = 0; i < encodedContent.length(); i++) {
        if (encodedContent[i] == '\\' && i + 1 < encodedContent.length()) {
            if (encodedContent[i + 1] == 'n') {
                content += '\n';
                i++;
            }
            else if (encodedContent[i + 1] == '\\') {
                content += '\\';
                i++;
            }
            else {
                content += encodedContent[i];
            }
        }
        else {
            content += encodedContent[i];
        }
    }
    return content;
}

// �Ӹ�Ŀ¼��ʼ����ÿ���ڵ�
void FileSystem::saveNode(std::ofstream& out, FileSystemNode* node, const std::string& prefix) {
    if (node == nullptr) return;

    // ����ڵ���Ϣ
    out << prefix << node->name << " "
        << (node->isDirectory ? "d" : "f") << " "
        << node->owner << " "
        << node->group << " "
        << node->userPermissions
        << node->groupPermissions
        << node->otherPermissions << " "
        << node->isBeingEdited << " "
        << encodeContent(node->content) << "\n";

    // �ݹ鱣���ӽڵ�
    for (auto& child : node->children) {
        saveNode(out, child.second, prefix + node->name + "/");
    }
}

// �����ļ�ϵͳ��Ӳ��ʵ��
void FileSystem::saveToFile(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    saveNode(out, root, "");
    out.close();
}



// �����û���Ϣ��Ӳ��ʵ��
void FileSystem::saveUsersToFile(const std::string& filename) {
    std::ofstream out(filename, std::ios::out);
    if (!out) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& userPair : users) {
        const User& user = userPair.second;
        out << user.username << " "
            << user.password << " "
            << user.group << " "
            << user.homeDirectory << "\n";
    }

    out.close();
}

// ��Ӳ�̼����ļ�ϵͳʵ��
void FileSystem::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);

    if (!in) {
        std::cerr << "Note: File not found, creating new file: " << filename << std::endl;
        // �������ļ�
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error creating file: " << filename << std::endl;
            return;
        }
        outFile.close();

        in.open(filename); // �ٴδ�
        if (!in) {
            std::cerr << "Error opening file after creating: " << filename << std::endl;
            return;
        }
    }

    // ��ȡÿһ��
    std::string line;

    while (std::getline(in, line)) {
        std::istringstream iss(line);

        std::string path, type, owner, group, permissions, content;
        bool isBeingEdited;

        iss >> path >> type >> owner >> group >> permissions >> isBeingEdited >> content;


        FileSystemNode* node = getNodeByPath(path);

        if (node) { // ���ڵ��Ƿ����
            // �����Ѵ��ڽڵ������
            node->owner = owner;
            node->group = group;
            node->setPermissions(permissions);
            node->isBeingEdited = isBeingEdited;

            if (type != "d") { // ����Ŀ¼����������
                node->content = decodeContent(content);
            }

        }
        else { // �����½ڵ�
            load(path, type == "d", owner, group);
            node = getNodeByPath(path);

            if (node) {
                node->setPermissions(permissions);
                node->isBeingEdited = isBeingEdited;

                if (type != "d") {
                    node->content = decodeContent(content);
                }
            }
        }
    }

    in.close();
}

// ��Ӳ�̼����û���Ϣʵ��
void FileSystem::loadUsersFromFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);
    if (!in) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::string username, password, group, homeDirectory;
    while (in >> username >> password >> group >> homeDirectory) {
        // ����û��Ƿ��Ѵ���
        if (users.find(username) == users.end()) {
            users.emplace(std::piecewise_construct,
                std::forward_as_tuple(username),
                std::forward_as_tuple(username, password, group, homeDirectory));
        }
    }
    in.close();
}

// ����Ȩ��ʵ��
void FileSystem::setPermissions(const std::string& path, const std::string& newPermissions) {
    FileSystemNode* node = getNodeByPath(path);
    if (!node) {
        std::cerr << "Error: File or directory does not exist\n";
        return;
    }
    node->setPermissions(newPermissions);

    saveToFile("filesystem_state.txt");
}

// ��ȡȨ��ʵ��
std::string FileSystem::getPermissions(const std::string& path) {
    FileSystemNode* node = getNodeByPath(path);
    if (!node) {
        std::cerr << "Error: File or directory does not exist\n";
        return "";
    }
    return node->getPermissions();
}

// �����û�ʵ��
void FileSystem::addUser(const std::string& username, const std::string& password, const std::string& group) {
    std::string homeDir = "/" + username;

    auto result = users.emplace(std::piecewise_construct,
        std::forward_as_tuple(username),
        std::forward_as_tuple(username, password, group, homeDir));

    if (result.second) { // ����ɹ�
        // ����Ŀ¼�Ƿ��Ѵ���
        if (!getNodeByPath(homeDir)) {
            load(homeDir, true, username, group);
        }
    }
    else {
        std::cerr << "Error: User already exists\n";
    }
}

// �����û�����ʵ��
void FileSystem::changeUserPassword(const std::string& username, const std::string& newPassword) {
    auto it = users.find(username);

    if (it == users.end()) {
        std::cerr << "Error: User does not exist\n";
        return;
    }

    it->second.password = newPassword;
    std::cout << "Password changed successfully for user: " << username << std::endl;
}

// ��¼ʵ��
bool FileSystem::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end() || it->second.password != password) {
        std::cerr << "Error: Invalid username or password\n";
        return false;
    }

    // ����û��Ѿ���¼��ֱ�ӷ���
    if (currentUser != nullptr && currentUser->username == username) {
        std::cout << "Already logged in as " << username << std::endl;
        return true;
    }

    currentUser = &it->second; // ���õ�ǰ�û�
    changeDirectory(currentUser->homeDirectory); // �л����û��ļ�Ŀ¼
    return true;
}

// ���Ȩ��ʵ��
bool FileSystem::checkPermissions(FileSystemNode* node, char permissionType) {
    // permissionType ����Ȩ������Ϊr,w,x
         if (currentUser->username == "root") {
             return true;  // root�û�ӵ������Ȩ��
         }

    std::string permissions = node->getPermissions();
    bool hasPermission = false;

    // ����û�Ȩ��
    if (node->owner == currentUser->username) {
        hasPermission =
            permissions[permissionType == 'r'
            ? 0
            : (permissionType == 'w' ? 1 : 2)] != '-';
    }
    // �����Ȩ��
    else if (node->group == currentUser->group) {
        hasPermission =
            permissions[permissionType == 'r'
            ? 3
            : (permissionType == 'w' ? 4 : 5)] != '-';
    }
    // ��������û�Ȩ��
    else {
        hasPermission =
            permissions[permissionType == 'r'
            ? 6
            : (permissionType == 'w' ? 7 : 8)] != '-';
    }
    return hasPermission;
}

void FileSystem::create(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group) {
    if (path.empty()) {
        std::cerr << "Error: Path cannot be empty\n";
        return;
    }

    // �������·��������ת��Ϊ����·��
    std::string absolutePath = path[0] == '/' ? path : getCurrentDirectoryPath() + path;

    // ���·���Ƿ����
    FileSystemNode* node = getNodeByPath(absolutePath);
    if (node) {
        std::cerr << "Error: File or directory already exists\n";
        return;
    }

    // ��ȡ��Ŀ¼·�����½ڵ�����
    size_t lastSlashPos = absolutePath.find_last_of('/');
    std::string parentPath = (lastSlashPos != std::string::npos) ? absolutePath.substr(0, lastSlashPos) : "/";
    std::string name = (lastSlashPos != std::string::npos) ? absolutePath.substr(lastSlashPos + 1) : absolutePath;


    FileSystemNode* parent = current; // ʹ�õ�ǰĿ¼��Ϊ���ڵ�

    // ȷ�����ڵ���һ����Ч��Ŀ¼
    if (!parent || !parent->isDirectory) {
        std::cerr << "Error: Invalid path\n";
        return;
    }

    // �����ļ���Ŀ¼
    FileSystemNode* newNode = new FileSystemNode(name, isDirectory, owner, group, "rwx", "r-x", "r-x", parent);
    
    // ���½ڵ���ӵ���Ŀ¼���ӽڵ�
    parent->children[name] = newNode;
}


void FileSystem::load(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group) {
    // ���·���Ƿ����
    FileSystemNode* node = getNodeByPath(path);
    if (node) {
        std::cerr << "Error: File or directory already exists\n";
        return;
    }

    // ��ȡ��Ŀ¼·�����½ڵ�����
    size_t lastSlashPos = path.find_last_of('/');  // �������б�ܵ�λ��
    std::string parentPath;
    std::string name;
    if (lastSlashPos != std::string::npos) {        // �ҵ�б��
        parentPath = path.substr(0, lastSlashPos);  // ��ȡ��Ŀ¼·��
        name = path.substr(lastSlashPos + 1);  // ��ȡ�½ڵ������
        if (parentPath.empty()) {              // ��Ŀ¼�Ǹ�Ŀ¼
            parentPath = "/";
        }
    }
    else {
        // û�ҵ�б��
        parentPath = "/";
        name = path;
    }

    // ��ȡ��Ŀ¼�ڵ�
    FileSystemNode* parent = getNodeByPath(parentPath);
    if (!parent || !parent->isDirectory) {
        std::cerr << "Error: Invalid path\n";
        return;
    }

    // �����ļ���Ŀ¼
    FileSystemNode* newNode = new FileSystemNode(
        name, isDirectory, owner, group, "rwx", "r-x", "r-x", parent);
    // ���½ڵ���ӵ���Ŀ¼���ӽڵ�
    parent->children[name] = newNode;
}

void FileSystem::remove(const std::string& path)
{
    if (path.empty()) {
        std::cerr << "Error: Path cannot be empty\n";
        return;
    }

    std::string absolutePath = path[0] == '/' ? path : getCurrentDirectoryPath() + path;

    size_t lastSlashPos = absolutePath.find_last_of('/');
    std::string parentPath = lastSlashPos != std::string::npos ? absolutePath.substr(0, lastSlashPos) : "/";
    std::string name = absolutePath.substr(lastSlashPos + 1);

    FileSystemNode* parent = getNodeByPath(parentPath);
    if (!parent || !parent->isDirectory || parent->children.find(name) == parent->children.end()) {
        std::cerr << "Error: File or directory does not exist\n";
        return;
    }

    delete parent->children[name];
    parent->children.erase(name);
}

void FileSystem::changeDirectory(const std::string& path)
{
    if (path.empty()) {
        std::cerr << "Error: Path cannot be empty\n";
        return;
    }

    std::string absolutePath = path[0] == '/' ? path : getCurrentDirectoryPath() + path;

    FileSystemNode* node = getNodeByPath(absolutePath);
    if (!node || !node->isDirectory) {
        std::cerr << "Error: Directory does not exist\n";
        return;
    }

    if (!checkPermissions(node, 'r')) {
        std::cerr << "Error: Permission denied\n";
        return;
    }

    current = node; // ���µ�ǰĿ¼
}

void FileSystem::listDirectory(bool longFormat)
{
    if (longFormat) {
        for (const auto& child : current->children) {
            std::cout << std::left << std::setw(10) << child.second->formatPermissions() << " " << child.first << "\n";
        }
    }
    else {
        for (const auto& child : current->children) {
            std::cout << (child.second->isDirectory ? "d " : "- ") << child.first << "\n";
        }
    }
}

void FileSystem::readFile(const std::string& path)
{
    if (path.empty()) {
        std::cerr << "Error: Path cannot be empty\n";
        return;
    }

    std::string absolutePath = path[0] == '/' ? path : getCurrentDirectoryPath() + path;

    FileSystemNode* node = getNodeByPath(absolutePath);
    if (!node || node->isDirectory) {
        std::cerr << "Error: Invalid file path\n";
        return;
    }

    if (!checkPermissions(node, 'r')) {
        std::cerr << "Error: Permission denied\n";
        return;
    }

    std::cout << node->content;
}

std::string FileSystem::getCurrentDirectoryPath()
{
    std::vector<std::string> parts;
    FileSystemNode* node = current;
    while (node != nullptr && node != root) {  // �ӵ�ǰ�ڵ�������
        parts.push_back(node->name);
        node = node->parent;
    }
    std::reverse(parts.begin(), parts.end());
    std::string path = "/";
    for (const auto& part : parts) {
        path += part + "/";
    }
    return path;
}
