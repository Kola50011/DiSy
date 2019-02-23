#pragma once

#include <string>
#include <experimental/filesystem>

#include "DiSy.pb.h"

#include <iostream>

namespace fs = std::experimental::filesystem;

namespace writer
{
void writeDirectory(std::string path, const DiSy::Directory *directory)
{
    std::cout << "Write " << path + directory->relative_path() << std::endl;
    // fs::create_directories(path + directory->relative_path());
}
} // namespace writer