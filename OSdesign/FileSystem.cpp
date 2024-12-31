#include "FileSystem.h"
#include <iomanip>

// 构造函数实现
FileSystem::FileSystem() {
    root = new FileSystemNode("/", true, "root", "root");
    current = root;

    home = new FileSystemNode("root", true, "root", "root", "rwx", "r-x", "r-x", root);
    root->children["root"] = home;

    users.emplace(std::piecewise_construct,
        std::forward_as_tuple("root"),
        std::forward_as_tuple("root", "password", "root", "/root"));
}

// 析构函数实现
FileSystem::~FileSystem() {
    delete root;
}

// 根据路径查找节点实现
FileSystemNode* FileSystem::getNodeByPath(const std::string& path) {
    if (path.empty()) return nullptr;

    std::string absolutePath = path;
    // 如果是相对路径，则将其转换为绝对路径
    if (path[0] != '/') {
        absolutePath = getCurrentDirectoryPath() + path;
    }

    // 继续原有的绝对路径处理逻辑
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

// 编码文件内容实现
std::string FileSystem::encodeContent(const std::string& content) {
    std::string encoded;
    for (char c : content) {
        if (c == '\n') {
            encoded += "\\n"; // 换行符替换
        }
        else if (c == '\\') {
            encoded += "\\\\"; // 反斜杠替换
        }
        else {
            encoded += c;
        }
    }
    return encoded;
}

// 解码文件内容实现
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

// 从根目录开始保存每个节点
void FileSystem::saveNode(std::ofstream& out, FileSystemNode* node, const std::string& prefix) {
    if (node == nullptr) return;

    // 保存节点信息
    out << prefix << node->name << " "
        << (node->isDirectory ? "d" : "f") << " "
        << node->owner << " "
        << node->group << " "
        << node->userPermissions
        << node->groupPermissions
        << node->otherPermissions << " "
        << node->isBeingEdited << " "
        << encodeContent(node->content) << "\n";

    // 递归保存子节点
    for (auto& child : node->children) {
        saveNode(out, child.second, prefix + node->name + "/");
    }
}

// 保存文件系统到硬盘实现
void FileSystem::saveToFile(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    saveNode(out, root, "");
    out.close();
}



// 保存用户信息到硬盘实现
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

// 从硬盘加载文件系统实现
void FileSystem::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);

    if (!in) {
        std::cerr << "Note: File not found, creating new file: " << filename << std::endl;
        // 创建新文件
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error creating file: " << filename << std::endl;
            return;
        }
        outFile.close();

        in.open(filename); // 再次打开
        if (!in) {
            std::cerr << "Error opening file after creating: " << filename << std::endl;
            return;
        }
    }

    // 读取每一行
    std::string line;

    while (std::getline(in, line)) {
        std::istringstream iss(line);

        std::string path, type, owner, group, permissions, content;
        bool isBeingEdited;

        iss >> path >> type >> owner >> group >> permissions >> isBeingEdited >> content;


        FileSystemNode* node = getNodeByPath(path);

        if (node) { // 检查节点是否存在
            // 更新已存在节点的属性
            node->owner = owner;
            node->group = group;
            node->setPermissions(permissions);
            node->isBeingEdited = isBeingEdited;

            if (type != "d") { // 不是目录，设置内容
                node->content = decodeContent(content);
            }

        }
        else { // 创建新节点
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

// 从硬盘加载用户信息实现
void FileSystem::loadUsersFromFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);
    if (!in) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::string username, password, group, homeDirectory;
    while (in >> username >> password >> group >> homeDirectory) {
        // 检查用户是否已存在
        if (users.find(username) == users.end()) {
            users.emplace(std::piecewise_construct,
                std::forward_as_tuple(username),
                std::forward_as_tuple(username, password, group, homeDirectory));
        }
    }
    in.close();
}

// 设置权限实现
void FileSystem::setPermissions(const std::string& path, const std::string& newPermissions) {
    FileSystemNode* node = getNodeByPath(path);
    if (!node) {
        std::cerr << "Error: File or directory does not exist\n";
        return;
    }
    node->setPermissions(newPermissions);

    saveToFile("filesystem_state.txt");
}

// 获取权限实现
std::string FileSystem::getPermissions(const std::string& path) {
    FileSystemNode* node = getNodeByPath(path);
    if (!node) {
        std::cerr << "Error: File or directory does not exist\n";
        return "";
    }
    return node->getPermissions();
}

// 新增用户实现
void FileSystem::addUser(const std::string& username, const std::string& password, const std::string& group) {
    std::string homeDir = "/" + username;

    auto result = users.emplace(std::piecewise_construct,
        std::forward_as_tuple(username),
        std::forward_as_tuple(username, password, group, homeDir));

    if (result.second) { // 插入成功
        // 检查家目录是否已存在
        if (!getNodeByPath(homeDir)) {
            load(homeDir, true, username, group);
        }
    }
    else {
        std::cerr << "Error: User already exists\n";
    }
}

// 更改用户密码实现
void FileSystem::changeUserPassword(const std::string& username, const std::string& newPassword) {
    auto it = users.find(username);

    if (it == users.end()) {
        std::cerr << "Error: User does not exist\n";
        return;
    }

    it->second.password = newPassword;
    std::cout << "Password changed successfully for user: " << username << std::endl;
}

// 登录实现
bool FileSystem::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end() || it->second.password != password) {
        std::cerr << "Error: Invalid username or password\n";
        return false;
    }

    // 如果用户已经登录，直接返回
    if (currentUser != nullptr && currentUser->username == username) {
        std::cout << "Already logged in as " << username << std::endl;
        return true;
    }

    currentUser = &it->second; // 设置当前用户
    changeDirectory(currentUser->homeDirectory); // 切换到用户的家目录
    return true;
}

// 检查权限实现
bool FileSystem::checkPermissions(FileSystemNode* node, char permissionType) {
    // permissionType 检查的权限类型为r,w,x
         if (currentUser->username == "root") {
             return true;  // root用户拥有所有权限
         }

    std::string permissions = node->getPermissions();
    bool hasPermission = false;

    // 检查用户权限
    if (node->owner == currentUser->username) {
        hasPermission =
            permissions[permissionType == 'r'
            ? 0
            : (permissionType == 'w' ? 1 : 2)] != '-';
    }
    // 检查组权限
    else if (node->group == currentUser->group) {
        hasPermission =
            permissions[permissionType == 'r'
            ? 3
            : (permissionType == 'w' ? 4 : 5)] != '-';
    }
    // 检查其他用户权限
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

    // 处理相对路径，将其转换为绝对路径
    std::string absolutePath = path[0] == '/' ? path : getCurrentDirectoryPath() + path;

    // 检查路径是否存在
    FileSystemNode* node = getNodeByPath(absolutePath);
    if (node) {
        std::cerr << "Error: File or directory already exists\n";
        return;
    }

    // 获取父目录路径和新节点名称
    size_t lastSlashPos = absolutePath.find_last_of('/');
    std::string parentPath = (lastSlashPos != std::string::npos) ? absolutePath.substr(0, lastSlashPos) : "/";
    std::string name = (lastSlashPos != std::string::npos) ? absolutePath.substr(lastSlashPos + 1) : absolutePath;


    FileSystemNode* parent = current; // 使用当前目录作为父节点

    // 确保父节点是一个有效的目录
    if (!parent || !parent->isDirectory) {
        std::cerr << "Error: Invalid path\n";
        return;
    }

    // 创建文件或目录
    FileSystemNode* newNode = new FileSystemNode(name, isDirectory, owner, group, "rwx", "r-x", "r-x", parent);
    
    // 将新节点添加到父目录的子节点
    parent->children[name] = newNode;
}


void FileSystem::load(const std::string& path, bool isDirectory, const std::string& owner, const std::string& group) {
    // 检查路径是否存在
    FileSystemNode* node = getNodeByPath(path);
    if (node) {
        std::cerr << "Error: File or directory already exists\n";
        return;
    }

    // 获取父目录路径和新节点名称
    size_t lastSlashPos = path.find_last_of('/');  // 查找最后斜杠的位置
    std::string parentPath;
    std::string name;
    if (lastSlashPos != std::string::npos) {        // 找到斜杠
        parentPath = path.substr(0, lastSlashPos);  // 提取父目录路径
        name = path.substr(lastSlashPos + 1);  // 提取新节点的名称
        if (parentPath.empty()) {              // 父目录是根目录
            parentPath = "/";
        }
    }
    else {
        // 没找到斜杠
        parentPath = "/";
        name = path;
    }

    // 获取父目录节点
    FileSystemNode* parent = getNodeByPath(parentPath);
    if (!parent || !parent->isDirectory) {
        std::cerr << "Error: Invalid path\n";
        return;
    }

    // 创建文件或目录
    FileSystemNode* newNode = new FileSystemNode(
        name, isDirectory, owner, group, "rwx", "r-x", "r-x", parent);
    // 将新节点添加到父目录的子节点
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

    current = node; // 更新当前目录
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
    while (node != nullptr && node != root) {  // 从当前节点向上找
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
