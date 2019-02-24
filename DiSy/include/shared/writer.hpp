#pragma once

#include <string>
#include <experimental/filesystem>

#include "DiSy.pb.h"

#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::experimental::filesystem;

namespace writer
{
void writeDirectory(std::string path, const DiSy::Directory *directory)
{
    fs::create_directories(path + directory->relative_path());
}

void writeFile(std::string path, const DiSy::File *file)
{
    std::string absolutePath = path + file->metadata().relative_path();
    std::ofstream stream;
    stream.open(absolutePath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

    stream << file->data();
    stream.close();
}
} // namespace writer