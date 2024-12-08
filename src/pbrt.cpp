// pbrt.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pbrt.h"
#include "api.h"

using namespace pbrt;

int main(int argc, char* argv[])
{
    Options options;
    std::vector<std::string> filenames;

    for (int i = 1; i < argc; ++i)
    {
        filenames.push_back(argv[i]);
    }

    pbrtInit(options);

    for (const auto& f : filenames)
        pbrtParseFile(f);
    
    pbrtCleanup();

    return 0;
}