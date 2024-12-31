#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "FileSystem.h"

class SimpleShell {
private:
    FileSystem fs;  // �ļ�ϵͳ����

    // �ָ��ַ���Ϊtokens�ĸ������� 
    std::vector<std::string> split(const std::string& str, char delimiter);

    // ��¼���� 
    void login(const std::vector<std::string>& args);

    // �޸��������� 
    void changePassword(const std::vector<std::string>& args);

    // �༭�ļ����� 
    void editFile(const std::string& filename);

    // Ȩ��ת�� 
    static std::string convertNumericToSymbolic(const std::string& numeric);

    // ִ������ 
    void executeCommand(const std::string& command, const std::vector<std::string>& args);

    //��ʾ����
    void help();



public:
    void run();  // ����shell����ѭ��
};
