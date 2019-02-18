#include "client/client.hpp"
#include "shared/reader.hpp"
#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

#include <string>

using namespace std;

void Client::downloadDirectories(google::protobuf::Map<string, DiSy::DirectoryMetadata> directories)
{
    for (auto &pair : directories)
    {
        downloadDirectory(pair.second);
    }
}

void Client::downloadDirectory(DiSy::DirectoryMetadata &directoryMetadata)
{
    grpc::ClientContext clientContext;
    DiSy::Directory directory;
    grpc::Status status{stub->GetDirectory(&clientContext, directoryMetadata, &directory)};

    if (status.ok())
    {
        console->info("DownloadDirectory success");
    }
    else
    {
        console->error("DownloadDirectory error: {}", status.error_message());
    }
}