#pragma once

#include "DiSy.pb.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

namespace reader
{
inline DiSy::Directory getDirectoryFromMetadata(DiSy::DirectoryMetadata &directoryMetadata)
{
    DiSy::Directory directory;
    directory.set_relative_path((&directoryMetadata)->relative_path());
    directory.set_allocated_metadata(&directoryMetadata);
    return directory;
}

inline DiSy::File getFileFromMetadata(DiSy::FileMetadata &fileMetadata, std::string path)
{
    DiSy::File file;
    file.set_allocated_metadata(&fileMetadata);

    std::ifstream stream(path + (&fileMetadata)->relative_path(), std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = stream.tellg();

    std::vector<char> fileData(pos);
    stream.seekg(0, std::ios::beg);
    stream.read(&fileData[0], pos);
    stream.close();

    file.set_data(fileData.data());

    return file;
}

} // namespace reader