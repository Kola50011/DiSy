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

    char byte;
    std::string data{};
    std::ifstream stream(path + (&fileMetadata)->relative_path(), std::ios_base::binary);
    while (stream)
    {
        stream.get(byte);
        data += byte;
    }
    stream.close();

    file.set_data(data);

    return file;
}

} // namespace reader