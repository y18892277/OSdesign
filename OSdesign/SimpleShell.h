#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "FileSystem.h"

class SimpleShell {
private:
    FileSystem fs;  // 文件系统对象

    // 分割字符串为tokens的辅助函数 
    std::vector<std::string> split(const std::string& str, char delimiter);

    // 登录命令 
    void login(const std::vector<std::string>& args);

    // 修改密码命令 
    void changePassword(const std::vector<std::string>& args);

    // 编辑文件命令 
    void editFile(const std::string& filename);

    // 权限转换 
    static std::string convertNumericToSymbolic(const std::string& numeric);

    // 执行命令 
    void executeCommand(const std::string& command, const std::vector<std::string>& args);

    //显示命令
    void help();



public:
    void run();  // 运行shell命令循环
};
