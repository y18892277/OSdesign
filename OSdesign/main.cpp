#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

// �û���
class User {
public:
    std::string username;       // �û���
    std::string password;       // ����
    std::string group;          // �û���
    std::string homeDirectory;  // ��Ŀ¼

    User(const std::string& username, const std::string& password,
        const std::string& group, const std::string& homeDirectory)
        : username(username),
        password(password),
        group(group),
        homeDirectory(homeDirectory) {
    }
};

// �ļ�ϵͳ�ڵ�
class FileSystemNode {
public:
    std::string name;     // �ļ���Ŀ¼��,������·��
    bool isDirectory;     // �Ƿ�ΪĿ¼
    std::string owner;    // ӵ����
    std::string group;    // �û���
    std::string content;  // �ļ�����
    std::map<std::string, FileSystemNode*> children;  // Ŀ¼����
    std::string userPermissions;                      // �û�Ȩ��
    std::string groupPermissions;                     // �û���Ȩ��
    std::string otherPermissions;                     // �����û�Ȩ��
    FileSystemNode* parent;  // ָ�򸸽ڵ��ָ��
    bool isBeingEdited;      // �༭״̬

    FileSystemNode(std::string name, bool isDirectory, std::string owner,
        std::string group,
        std::string userPermissions = "rwx",  // Ĭ���û�Ȩ��
        std::string groupPermissions = "r-x",  // Ĭ���û���Ȩ��
        std::string otherPermissions = "r-x",  // Ĭ�������û�Ȩ��
        FileSystemNode* parent = nullptr)
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

    ~FileSystemNode() {
        for (auto& child : children) {
            delete child.second;
        }
    }

    // ����Ȩ��
    void setPermissions(const std::string& newPermissions) {
        // newPermissions ��ʽΪ "rwxr-xr-x"
        if (newPermissions.size() == 9) {
            userPermissions = newPermissions.substr(0, 3);
            groupPermissions = newPermissions.substr(3, 3);
            otherPermissions = newPermissions.substr(6, 3);
        }
    }

    // ��ȡȨ��
    std::string getPermissions() const {
        return userPermissions + groupPermissions + otherPermissions;
    }

    // ��ʽ���ļ���Ŀ¼��Ȩ�ޱ�ʾ
    std::string formatPermissions() const {
        std::ostringstream oss;
        oss << (isDirectory ? 'd' : '-') << userPermissions << groupPermissions
            << otherPermissions << " " << owner << " " << group;
        return oss.str();
    }
};

// �ļ�ϵͳ��
class FileSystem {
private:
    FileSystemNode* root;                         // ��Ŀ¼
    FileSystemNode* current;                      // ����Ŀ¼
    FileSystemNode* home;                         // ��Ŀ¼
    std::unordered_map<std::string, User> users;  // �����û�

public:
    User* currentUser = nullptr;  // ��ǰ��¼���û�

    FileSystem() {
        // �½���Ŀ¼
        root = new FileSystemNode("/", true, "root", "root");
        current = root;
        // �½���Ŀ¼
        home = new FileSystemNode("root", true, "root", "root", "rwx", "r-x",
            "r-x", root);
        root->children["root"] = home;
        // ���root�û�
        users.emplace(
            std::piecewise_construct, std::forward_as_tuple("root"),
            std::forward_as_tuple("root", "password", "root", "/root"));
        // currentUser = &users.find("root")->second;  // Ĭ��root��¼
    }

    ~FileSystem() { delete root; }

    // ����·�����ҽڵ�
    FileSystemNode* getNodeByPath(const std::string& path) {
        if (path == "/") return root;

        // ��֤·����ʽ
        if (path.empty() || path[0] != '/') {
            std::cerr << "Error: Only absolute paths are supported.\n";
            return nullptr;
        }

        // �ֽ�·��
        std::vector<std::string> parts;
        std::istringstream iss(path);
        std::string part;
        while (std::getline(iss, part, '/')) {  // �ָ�·��
            if (!part.empty()) {                // ����β��б��
                parts.push_back(part);
            }
        }

        // ����·��
        FileSystemNode* node = root;
        for (const auto& p : parts) {
            if (node->children.find(p) == node->children.end()) return nullptr;
            node = node->children[p];
        }
        return node;
    }
    // �����ļ����ݣ�ת�廻�з�
    static std::string encodeContent(const std::string& content) {
        std::string encoded;
        for (char c : content) {
            if (c == '\n') {
                encoded += "\\n";  // ���з��滻
            }
            else if (c == '\\') {
                encoded += "\\\\";  // ��б���滻
            }
            else {
                encoded += c;
            }
        }
        return encoded;
    }

    // �����ļ�����
    static std::string decodeContent(const std::string& encodedContent) {
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

    // �����ļ�ϵͳ��Ӳ��
    void saveToFile(const std::string& filename) {
        std::ofstream out(filename);
        if (!out) {
            std::cerr << "Error opening file for writing: " << filename
                << std::endl;
            return;
        }
        saveNode(out, root, "");
        out.close();
    }

    // �Ӹ�Ŀ¼��ʼ����ÿ���ڵ�
    void saveNode(std::ofstream& out, FileSystemNode* node,
        const std::string& prefix) {
        if (node == nullptr) return;

        // ����ڵ���Ϣ
        // prefix: ·��ǰ׺
        out << prefix << node->name << " " << (node->isDirectory ? "d" : "f")
            << " " << node->owner << " " << node->group << " "
            << node->userPermissions << node->groupPermissions
            << node->otherPermissions << " " << node->isBeingEdited << " "
            << encodeContent(node->content) << "\n";

        // �ݹ鱣���ӽڵ�
        for (auto& child : node->children) {
            saveNode(out, child.second, prefix + node->name + "/");
        }
    }

    // �����û�
    void saveUsersToFile(const std::string& filename) {
        std::ofstream out(filename, std::ios::out);
        if (!out) {
            std::cerr << "Error opening file for writing: " << filename
                << std::endl;
            return;
        }

        for (const auto& userPair : users) {
            const User& user = userPair.second;
            out << user.username << " " << user.password << " " << user.group
                << " " << user.homeDirectory << "\n";
        }

        out.close();
    }

    // ��Ӳ�̼����ļ�ϵͳ
    void loadFromFile(const std::string& filename) {
        std::ifstream in(filename);

        if (!in) {
            std::cerr << "Note: File not found, creating new file: " << filename
                << std::endl;

            // �������ļ�
            std::ofstream outFile(filename);
            if (!outFile) {
                std::cerr << "Error creating file: " << filename << std::endl;
                return;
            }

            // �ر��´������ļ������´�
            outFile.close();
            in.open(filename);

            // �ٴ�ʧ�ܣ���������
            if (!in) {
                std::cerr << "Error opening file after creating: " << filename
                    << std::endl;
                return;
            }
        }

        // ��ȡÿһ��
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string path, type, owner, group, permissions, content;
            bool isBeingEdited;
            iss >> path >> type >> owner >> group >> permissions >>
                isBeingEdited >> content;

            FileSystemNode* node = getNodeByPath(path);
            if (node) {  // ���ڵ��Ƿ����
                // �����Ѵ��ڽڵ������
                node->owner = owner;
                node->group = group;
                node->setPermissions(permissions);
                node->isBeingEdited = isBeingEdited;
                if (type != "d") {  // ����Ŀ¼,��������
                    node->content = decodeContent(content);
                }
            }
            else {
                // �����½ڵ�
                create(path, type == "d", owner, group);
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

    void loadUsersFromFile(const std::string& filename) {
        std::ifstream in(filename, std::ios::in);
        if (!in) {
            std::cerr << "Error opening file for reading: " << filename
                << std::endl;
            return;
        }

        std::string username, password, group, homeDirectory;
        while (in >> username >> password >> group >> homeDirectory) {
            users.emplace(std::piecewise_construct,
                std::forward_as_tuple(username),
                std::forward_as_tuple(username, password, group,
                    homeDirectory));
            // users[username] = User(username, password, group, homeDirectory);
        }

        in.close();
    }

    // ����Ȩ��
    void setPermissions(const std::string& path,
        const std::string& newPermissions) {
        FileSystemNode* node = getNodeByPath(path);
        if (!node) {
            std::cerr << "Error: File or directory does not exist\n";
            return;
        }
        node->setPermissions(newPermissions);
    }

    // ��ȡȨ��
    std::string getPermissions(const std::string& path) {
        FileSystemNode* node = getNodeByPath(path);
        if (!node) {
            std::cerr << "Error: File or directory does not exist\n";
            return "";
        }
        return node->getPermissions();
    }

    // �����û�
    void addUser(const std::string& username, const std::string& password,
        const std::string& group) {
        std::string homeDir = "/" + username;
        auto result = users.emplace(
            std::piecewise_construct, std::forward_as_tuple(username),
            std::forward_as_tuple(username, password, group, homeDir));
        if (result.second) {  // ����ɹ�
            // ����Ŀ¼�Ƿ��Ѵ���
            if (!getNodeByPath(homeDir)) {
                // ������Ŀ¼
                create(homeDir, true, username, group);
            }
        }
        else {
            std::cerr << "Error: User already exists\n";
        }
    }

    void changeUserPassword(const std::string& username,
        const std::string& newPassword) {
        auto it = users.find(username);
        if (it == users.end()) {
            std::cerr << "Error: User does not exist\n";
            return;
        }
        it->second.password = newPassword;
        std::cout << "Password changed successfully for user: " << username
            << std::endl;
    }

    bool login(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end() || it->second.password != password) {
            std::cerr << "Error: Invalid username or password\n";
            return false;
        }
        currentUser = &it->second;
        changeDirectory(currentUser->homeDirectory);
        return true;
    }

    // ���Ȩ��
    bool checkPermissions(FileSystemNode* node, char permissionType) {
        // permissionType ����Ȩ������Ϊr,w,x
        // if (currentUser->username == "root") {
        //     return true;  // root�û�ӵ������Ȩ��
        // }
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

    // �������ļ���Ŀ¼
    void create(const std::string& path, bool isDirectory,
        const std::string& owner, const std::string& group) {
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

    // ɾ���ļ���Ŀ¼
    void remove(const std::string& path) {
        if (path.empty() || path[0] != '/') {
            std::cerr << "Error: Only absolute paths are supported\n";
            return;
        }

        size_t lastSlashPos = path.find_last_of('/');
        std::string parentPath;  // ��Ŀ¼·��
        if (lastSlashPos == 0) {
            // ɾ�����Ǹ�Ŀ¼�µ��ļ���Ŀ¼
            parentPath = "/";
        }
        else {
            parentPath = path.substr(0, lastSlashPos);
        }
        std::string name = path.substr(lastSlashPos + 1);  // �ڵ���
        FileSystemNode* parent = getNodeByPath(parentPath);

        if (!parent || !parent->isDirectory ||
            parent->children.find(name) == parent->children.end()) {
            std::cerr << "Error: File or directory does not exist\n";
            return;
        }

        delete parent->children[name];  // ɾ������
        parent->children.erase(name);   // ɾ��map�ļ�ֵ��
    }

    // ���ĵ�ǰ����Ŀ¼
    void changeDirectory(const std::string& path) {
        if (path.empty() || path[0] != '/') {
            std::cerr << "Error: Only absolute paths are supported\n";
            return;
        }

        FileSystemNode* node = getNodeByPath(path);
        if (!node || !node->isDirectory) {
            std::cerr << "Error: Directory does not exist\n";
            return;
        }

        // ����Ƿ��ж�Ȩ��
        if (!checkPermissions(node, 'r')) {
            std::cerr << "Error: Permission denied\n";
            return;
        }

        current = node;
    }

    // �г���ǰĿ¼������
    void listDirectory(bool longFormat = false) {
        if (longFormat) {  // ls -l
            for (const auto& child : current->children) {
                std::cout << std::left << std::setw(10)
                    << child.second->formatPermissions() << " "
                    << child.first << "\n";
            }
        }
        else {  // ls
            for (const auto& child : current->children) {
                std::cout << (child.second->isDirectory ? "d " : "- ")
                    << child.first << "\n";  // d:Ŀ¼,-:�ļ�
            }
        }
    }

    // ��ȡ�ļ�����
    void readFile(const std::string& path) {
        FileSystemNode* node = getNodeByPath(path);
        if (!node || node->isDirectory) {
            std::cerr << "Error: Invalid file path\n";
            return;
        }

        // ����Ƿ��ж�Ȩ��
        if (!checkPermissions(node, 'r')) {
            std::cerr << "Error: Permission denied\n";
            return;
        }

        // ����ļ��Ƿ����ڱ��༭
        loadFromFile("filesystem_state.txt");
        if (node->isBeingEdited) {
            std::cerr << "Error: File is currently being edited\n";
            return;
        }

        std::cout << node->content;
    }

    // ��ȡ��ǰ����Ŀ¼�ľ���·��
    std::string getCurrentDirectoryPath() {
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
};

class SimpleShell {
private:
    FileSystem fs;

    // �ָ��ַ���
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            if (!token.empty()) tokens.push_back(token);
        }
        return tokens;
    }

    // login����
    void login(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cerr << "Usage: login <username> <password>" << std::endl;
            return;
        }
        if (!fs.login(args[0], args[1])) {
            std::cerr << "Login failed for user: " << args[0] << std::endl;
        }
        else {
            std::cout << "Logged in as " << args[0] << std::endl;
        }
    }

    void changePassword(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cerr << "Usage: passwd <username> <newpassword>" << std::endl;
            return;
        }
        fs.changeUserPassword(args[0], args[1]);
    }

    // vim����
    void editFile(const std::string& filename) {
        FileSystemNode* fileNode = fs.getNodeByPath(filename);
        if (!fileNode) {
            std::cout << "File does not exist. Creating new file: " << filename
                << std::endl;
            // �������򴴽��ļ�
            fs.create(filename, false, fs.currentUser->username,
                fs.currentUser->group);
            fileNode = fs.getNodeByPath(filename);
        }
        if (fileNode->isDirectory) {
            std::cerr << "Error: Path is a directory\n";
            return;
        }
        // ���дȨ��
        if (!fs.checkPermissions(fileNode, 'w')) {
            std::cerr << "Error: Permission denied.\n";
            return;
        }
        // ��ȡ��д״̬
        fs.loadFromFile("filesystem_state.txt");
        if (fileNode->isBeingEdited) {
            std::cerr << "Error: File is currently being edited\n";
            return;
        }
        fileNode->isBeingEdited = true;
        fs.saveToFile("filesystem_state.txt");

        // ��ʾ�ļ����� �������ģʽ
        std::cout << "Entering insert mode for file: " << filename << std::endl;
        std::cout << fileNode->content;

        // �༭�ļ�����
        std::string newContent = fileNode->content;
        std::string inputLine;
        while (std::getline(std::cin, inputLine)) {
            if (inputLine == ":wq") {
                // ���沢�˳�
                fileNode->content = newContent;
                std::cout << "File saved and exiting editor." << std::endl;
                break;
            }
            else {
                // �������
                newContent += inputLine + "\n";
            }
        }
        fileNode->isBeingEdited = false;
    }

    // ����Ȩ��ת��Ϊ�ַ�Ȩ��
    std::string convertNumericToSymbolic(const std::string& numeric) {
        if (numeric.size() != 3) return "";

        std::string symbolic;
        const std::string rwx[8] = { "---", "--x", "-w-", "-wx",
                                    "r--", "r-x", "rw-", "rwx" };

        for (char c : numeric) {
            if (c < '0' || c > '7') return "";  // �Ƿ�����
            symbolic += rwx[c - '0'];
        }

        return symbolic;
    }

    void executeCommand(const std::string& command,
        const std::vector<std::string>& args) {
        if (command == "login") {
            login(args);
        }
        else {
            // ����û���¼
            if (fs.currentUser == nullptr) {
                std::cerr << "Error: No user logged in" << std::endl;
                return;
            }
            else if (command == "chmod") {
                if (args.size() != 2) {
                    std::cerr
                        << "Usage: chmod <permissions> <filename/directory>"
                        << std::endl;
                    return;
                }
                // ������Ȩ��ת��Ϊ�ַ�Ȩ��
                std::string symbolicPermissions =
                    convertNumericToSymbolic(args[0]);
                if (symbolicPermissions.empty()) {
                    std::cerr << "Invalid numeric permissions: " << args[0]
                        << std::endl;
                    return;
                }
                fs.setPermissions(args[1], symbolicPermissions);
            }
            else if (command == "getperm") {
                if (args.size() != 1) {
                    std::cerr << "Usage: getperm <filename/directory>"
                        << std::endl;
                    return;
                }
                std::cout << fs.getPermissions(args[0]) << std::endl;
            }
            else if (command == "pwd") {
                std::cout << fs.getCurrentDirectoryPath() << std::endl;
            }
            else if (command == "cd") {
                if (args.size() == 1) {
                    fs.changeDirectory(args[0]);
                }
                else {
                    std::cerr << "Usage: cd [directory]" << std::endl;
                    return;
                }
            }
            else if (command == "ls") {
                bool longFormat = !args.empty() && args[0] == "-l";
                fs.listDirectory(longFormat);
            }
            else if (command == "touch") {
                if (args.size() != 1) {
                    std::cerr << "Usage: touch <filename>" << std::endl;
                    return;
                }
                fs.create(args[0], false, fs.currentUser->username,
                    fs.currentUser->group);
            }
            else if (command == "rm") {
                if (args.size() != 1) {
                    std::cerr << "Usage: rm <filename/directory>" << std::endl;
                    return;
                }
                fs.remove(args[0]);
            }
            else if (command == "mkdir") {
                if (args.size() != 1) {
                    std::cerr << "Usage: mkdir <directory>" << std::endl;
                    return;
                }
                fs.create(args[0], true, fs.currentUser->username,
                    fs.currentUser->group);
            }
            else if (command == "rmdir") {
                if (args.size() != 1) {
                    std::cerr << "Usage: rmdir <directory>" << std::endl;
                    return;
                }
                fs.remove(args[0]);
            }
            else if (command == "cat") {
                if (args.size() != 1) {
                    std::cerr << "Usage: cat <filename>" << std::endl;
                    return;
                }
                fs.readFile(args[0]);
            }
            else if (command == "vim") {
                if (args.size() != 1) {
                    std::cerr << "Usage: vim <filename>" << std::endl;
                    return;
                }
                editFile(args[0]);
            }
            else if (command == "adduser") {
                if (args.size() != 2) {
                    std::cerr << "Usage: adduser <username> <password>"
                        << std::endl;
                    return;
                }
                fs.addUser(args[0], args[1], "users");  // Ĭ���û���Ϊ"users"
            }
            else if (command == "su") {
                if (args.size() != 2) {
                    std::cerr << "Usage: su <username> <password>" << std::endl;
                    return;
                }
                if (!fs.login(args[0], args[1])) {
                    std::cerr << "Switch user failed for user: " << args[0]
                        << std::endl;
                }
                else {
                    std::cout << "Switched to user " << args[0] << std::endl;
                }
            }
            else if (command == "passwd") {
                if (args.size() != 2) {
                    std::cerr << "Usage: passwd <username> <newpassword>"
                        << std::endl;
                    return;
                }
                fs.changeUserPassword(args[0], args[1]);
            }
            else {
                std::cerr << "Unknown command: " << command << std::endl;
            }
        }
    }

public:
    void run() {
        std::string line;
        while (true) {
            fs.loadFromFile("filesystem_state.txt");
            fs.loadUsersFromFile("users.txt");

            std::cout << fs.getCurrentDirectoryPath()
                << "$ ";  // ��ǰ����Ŀ¼����ʾ��
            std::getline(std::cin, line);

            if (line == "exit") {
                fs.saveToFile("filesystem_state.txt");
                fs.saveUsersToFile("users.txt");
                std::cout << "logout" << std::endl;
                break;
            }
            std::vector<std::string> tokens = split(line, ' ');
            if (tokens.empty()) continue;

            std::string command = tokens[0];
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            executeCommand(command, args);
            fs.saveToFile("filesystem_state.txt");
            fs.saveUsersToFile("users.txt");
        }
    }
};

int main() {
    SimpleShell shell;
    shell.run();
    return 0;
}