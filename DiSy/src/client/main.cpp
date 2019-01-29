#include "CLI11.hpp"

#include "client/crawler.hpp"

using namespace std;

void eraseSubStr(std::string &mainStr, const std::string &toErase)
{
    size_t pos = mainStr.find(toErase);
    if (pos != string::npos)
    {
        mainStr.erase(pos, toErase.length());
    }
}

int main(int argc, char const *argv[])
{
    CLI ::App app{"DiSy server"};

    std::string path = "default";
    app.add_option("-d,--dir", path, "Directory to synchronize")->required()->check(CLI::ExistingDirectory);
    CLI11_PARSE(app, argc, argv);

    crawler::crawlDirectory(path);
    return 0;
}
