#pragma once

#include <string>
#include <experimental/filesystem>
#include <iostream>
#include <sys/stat.h>

#include "DiSy.pb.h"
#include "hashing.hpp"

namespace fs = std::experimental::filesystem;

namespace crawler
{
DiSy::DirTree *crawlDirectory(std::string path)
{
    DiSy::DirTree *dirTree = new DiSy::DirTree();

    size_t pathSize{path.size()};
    for (auto &p : fs::recursive_directory_iterator(path))
    {
        if (!fs::is_directory(p))
        {
            DiSy::FileMetadata *fileMetadata = dirTree->add_files();

            struct stat fileStats;
            stat(p.path().string().c_str(), &fileStats);

            fileMetadata->set_relative_path(p.path().string().substr(pathSize));
            fileMetadata->set_hash(hashing::getFileHash(p.path().string().c_str()));
            fileMetadata->set_last_modified_time(fileStats.st_mtime);
        }
        else
        {
            DiSy::DirectoryMetadata *directoryMetadata = dirTree->add_directories();
            directoryMetadata->set_relative_path(p.path().string().substr(pathSize));
        }
    }

    return dirTree;
}
} // namespace crawler
