#include "client/client.hpp"
#include "shared/reader.hpp"
#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

#include <string>

using namespace std;

void Client::uploadDirectories(google::protobuf::Map<string, DiSy::DirectoryMetadata> directories)
{
    for (auto &pair : directories)
    {
        uploadDirectory(pair.second);
    }
}

void Client::uploadDirectory(DiSy::DirectoryMetadata &directoryMetadata)
{
    grpc::ClientContext clientContext;
    DiSy::Directory directory = reader::getDirectoryFromMetadata(directoryMetadata);
    DiSy::Empty empty;
    grpc::Status status{stub->SendDirectory(&clientContext, directory, &empty)};

    if (status.ok())
    {
        console->info("UploadDirectory success");
    }
    else
    {
        console->error("UploadDirectory error: {}", status.error_message());
    }
}