#include "CLI11.hpp"
#include <experimental/filesystem>
#include <iostream>

using namespace std;
namespace fs = std::experimental::filesystem;

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

    size_t pathSize{path.size()};
    for (auto &p : fs::recursive_directory_iterator(path))
    {
        if (!fs::is_directory(p))
        {
            cout << p.path().string().substr(pathSize) << endl;
            // file->set_date(std::chrono::system_clock::to_time_t(fs::last_write_time(p)));
        }
    }

    return 0;
}
