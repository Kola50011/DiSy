#pragma once

#include "DiSy.pb.h"

namespace reader
{
DiSy::Directory getDirectoryFromMetadata(DiSy::DirectoryMetadata &directoryMetadata)
{
    DiSy::Directory directory;
    directory.set_relative_path((&directoryMetadata)->relative_path());
    directory.set_allocated_metadata(&directoryMetadata);
    directory.release_metadata(); // without this grpc takes ownership of object -> objects gets deleted too early
    return directory;
}

} // namespace reader