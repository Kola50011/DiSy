#pragma once

#include <string>
#include <experimental/filesystem>

#include "DiSy.pb.h"

#include <iostream>
#include <fstream>
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
    std::fstream stream(path + file->metadata().relative_path(), std::ios::binary | std::ios::out);

    stream.seekg(0, std::ios::beg);
    stream.write(file->data().data(), file->data().size());
    stream.close();
}
} // namespace writer