#include "SimpleShell.h"

// 分割字符串为tokens的辅助函数 实现 
std::vector<std::string> SimpleShell::split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) tokens.push_back(token);
	}

	return tokens;
}

void SimpleShell::run()
{
	std::string line;
	while (true) {
		fs.loadFromFile("filesystem_state.txt");
		fs.loadUsersFromFile("users.txt");

		std::cout << fs.getCurrentDirectoryPath()
			<< "$ ";  // 当前工作目录和提示符
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

// 登录命令 实现  
void SimpleShell::login(const std::vector<std::string>& args) {
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

// 修改密码命令 实现  
void SimpleShell::changePassword(const std::vector<std::string>& args) {
	if (args.size() != 2) {
		std::cerr << "Usage: passwd <username> <newpassword>" << std::endl;
		return;
	}
	fs.changeUserPassword(args[0], args[1]);
}

// 编辑文件命令 实现  
void SimpleShell::editFile(const std::string& filename) {
	FileSystemNode* fileNode = fs.getNodeByPath(filename);

	if (!fileNode) {
		std::cout << "File does not exist. Creating new file: " << filename << std::endl;
		fs.create(filename, false, fs.currentUser->username, fs.currentUser->group);
		fileNode = fs.getNodeByPath(filename);
	}

	if (fileNode->isDirectory) {
		std::cerr << "Error: Path is a directory\n";
		return;
	}

	if (!fs.checkPermissions(fileNode, 'w')) {
		std::cerr << "Error: Permission denied.\n";
		return;
	}

	fs.loadFromFile("filesystem_state.txt");

	if (fileNode->isBeingEdited) {
		std::cerr << "Error: File is currently being edited\n";
		return;
	}

	fileNode->isBeingEdited = true;

	fs.saveToFile("filesystem_state.txt");

	std::cout << "Entering insert mode for file: " << filename << std::endl;
	std::cout << fileNode->content;

	std::string newContent = fileNode->content;
	std::string inputLine;

	while (std::getline(std::cin, inputLine)) {
		if (inputLine == ":wq") {  // 保存并退出   
			fileNode->content = newContent;
			std::cout << "File saved and exiting editor." << std::endl;
			break;
		}
		else {  // 添加新行   
			newContent += inputLine + "\n";
		}
	}

	fileNode->isBeingEdited = false;
}

// 权限转换为字符权限 实现  
std::string SimpleShell::convertNumericToSymbolic(const std::string & numeric) {
	if (numeric.size() != 3) return "";

std::string symbolic;
const std::string rwx[8] = { "---","--x","-w-","-wx","r--","r-x","rw-","rwx" };

	for (char c : numeric) {
		if (c < '0' || c>'7') return "";  // 非法数字   
		symbolic += rwx[c - '0'];
	}

	return symbolic;
}

// 执行命令 实现    
void SimpleShell::executeCommand(const std::string & command, const std::vector<std::string>& args) {
	if (command == "login") {
		login(args);
	}
	else {
		if (fs.currentUser == nullptr) {
		std::cerr << " Error: No user logged in" << std::endl;
			return;
		}
		else if (command == "chmod") {
			if (args.size() != 2) {
			std::cerr << " Usage: chmod <permissions> <filename/directory>" << std::endl;
				return;
			}

			// 将数字权限转换为字符权限    
		std::string symbolicPermissions = convertNumericToSymbolic(args[0]);
			if (symbolicPermissions.empty()) {
				std::cerr << " Invalid numeric permissions: " << args[0] << std::endl;
				return;
			}

			fs.setPermissions(args[1], symbolicPermissions);
		}
		else if (command == "getperm") {
			if (args.size() != 1) {
				std::cerr << " Usage: getperm <filename/directory>" << std::endl;
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
				std::cerr << " Usage: cd [directory]" << std::endl;
				return;
			}
		}
		else if (command == "ls") {
			bool longFormat = !args.empty() && args[0] == "-l";
			fs.listDirectory(longFormat);
		}
		else if (command == "touch") {
			if (args.size() != 1) {
				std::cerr << " Usage: touch <filename>" << std::endl;
				return;
			}

			fs.create(args[0], false, fs.currentUser->username, fs.currentUser->group);
		}
		else if (command == "rm") {
			if (args.size() != 1) {
				std::cerr << " Usage: rm <filename/directory>" << std::endl;
				return;
			}

			fs.remove(args[0]);
		}
		else if (command == "mkdir") {
			if (args.size() != 1) {
				std::cerr << " Usage: mkdir <directory>" << std::endl;
				return;
			}

			fs.create(args[0], true, fs.currentUser->username, fs.currentUser->group);
		}
		else if (command == "rmdir") {
			if (args.size() != 1) {
				std::cerr << " Usage: rmdir <directory>" << std:: endl;
				return;
			}

			fs.remove(args[0]);
		}
		else if (command == "cat") {
			if (args.size() != 1) {
				std :: cerr << " Usage: cat <filename>" << std::endl;
				return;
			}

			fs.readFile(args[0]);
		}
		else if (command == "vim") {
			if (args.size() != 1) {
				std::cerr << " Usage: vim <filename>" << std::endl;
				return;
			}

			editFile(args[0]);
		}
		else if (command == "adduser") {
			if (args.size() != 2) {
				std::cerr << " Usage: adduser <username> <password>" << std::endl;
				return;
			}

			fs.addUser(args[0], args[1], "users");  // 默认用户组为"users"      
		}
		else if (command == "su") {
			if (args.size() != 2) {
				std:: cerr << " Usage: su <username> <password>" << std:: endl;
				return;
			}

			if (!fs.login(args[0], args[1])) {
				std :: cerr << " Switch user failed for user: " << args[0] << std :: endl;
			}
			else {
				std :: cout << " Switched to user " << args[0] << std :: endl;
			}
		}
		else if (command == "passwd") {
			if (args.size() != 2) {
				std :: cerr << " Usage: passwd <username> <newpassword>" << std :: endl;
				return;
			}

			fs.changeUserPassword(args[0], args[1]);
		}
		else if (command == "help") {
			help();
		}
		else {
			std :: cerr << " Unknown command: " << command << std :: endl;
		}
	}


}

void SimpleShell::help() {
	std::cout << "Available commands:\n";
	std::cout << "1. login <username> <password> - Log in as a user\n";
	std::cout << "2. passwd <username> <newpassword> - Change user password\n";
	std::cout << "3. chmod <permissions> <filename/directory> - Change permissions\n";
	std::cout << "4. getperm <filename/directory> - Get permissions of a file/directory\n";
	std::cout << "5. pwd - Print current working directory\n";
	std::cout << "6. cd <directory> - Change current directory\n";
	std::cout << "7. ls [-l] - List files in the current directory\n";
	std::cout << "8. touch <filename> - Create a new file\n";
	std::cout << "9. rm <filename/directory> - Remove a file or directory\n";
	std::cout << "10. mkdir <directory> - Create a new directory\n";
	std::cout << "11. rmdir <directory> - Remove a directory\n";
	std::cout << "12. cat <filename> - Display content of a file\n";
	std::cout << "13. vim <filename> - Edit a file using vim-like interface\n";
	std::cout << "14. adduser <username> <password> - Add a new user\n";
	std::cout << "15. su <username> <password> - Switch user\n";
	std::cout << "16. exit - Exit the shell\n";
}

