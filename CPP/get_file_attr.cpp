#include <sys/types.h>
#include <sys/extattr.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

// clang++ -std=c++14 attr.cpp -o attr
// ./attr 文件名，看有多少个attribute
// ./attr 文件名 attribute，看attribute的值
// ./attr 文件名 attribute 值，修改attribute的值
int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        ssize_t len = extattr_list_file(argv[1], EXTATTR_NAMESPACE_USER, NULL, 0);
        char* buffer = reinterpret_cast<char*>(malloc(len));
        extattr_list_file(argv[1], EXTATTR_NAMESPACE_USER, buffer, len);
        auto index = 0;
        std::vector<std::string> attrs;
        while (index < len)
        {
            attrs.emplace_back(&buffer[index + 1], buffer[index]);
            index += buffer[index] + 1;
        }
        for (auto& attr : attrs)
        {
            std::cout << attr << std::endl;
        }
    }
    else if (argc == 3)
    {
        ssize_t len = extattr_get_file(argv[1], EXTATTR_NAMESPACE_USER, argv[2], NULL, 0);
        char* buffer = new char[len];
        extattr_get_file(argv[1], EXTATTR_NAMESPACE_USER, argv[2], buffer, len);
        std::cout << buffer << std::endl;
    }
    else if (argc == 4)
    {
        extattr_set_file(argv[1], EXTATTR_NAMESPACE_USER, argv[2], argv[3], strlen(argv[3]));
    }
}