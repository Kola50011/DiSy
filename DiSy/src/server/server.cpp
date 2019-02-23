#include <grpcpp/grpcpp.h>
#include <iostream>
#include <math.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"
#include "server/server.hpp"
#include "shared.hpp"
#include "shared/crawler.hpp"
#include "shared/writer.hpp"
#include "shared/reader.hpp"

using namespace std;

Server::Server(string _path)
{
    serverDirTree = crawler::crawlDirectory(_path);
    path = _path;
}

bool areEqual(DiSy::DirectoryMetadata &firstDirectory, DiSy::DirectoryMetadata &secondDirectory)
{
    if (firstDirectory.relative_path() == secondDirectory.relative_path())
    {
        return true;
    }
    return false;
}

grpc::Status Server::GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                              DiSy::GetNewIdResponse *getNewIdResponse)
{
    if (!context || !empty)
    {
        return grpc::Status::CANCELLED;
    }
    getNewIdResponse->set_id(nextClientId);
    nextClientId += 1;
    return grpc::Status::OK;
}

bool Server::checkClientApproved(const DiSy::UpdateRequest *updateRequest)
{
    uint32_t currentServerTime = shared::getCurrentTime();
    uint32_t currentClientTime = updateRequest->time();

    if (max(currentServerTime, currentClientTime) - min(currentServerTime, currentClientTime) < 1000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Server::processUpdate(const DiSy::UpdateRequest *updateRequest,
                           DiSy::UpdateResponse *updateResponse)
{
    if (!updateResponse)
    {
        return;
    }

    DiSy::DirTree *uploads = new DiSy::DirTree();
    DiSy::DirTree *downloads = new DiSy::DirTree();

    auto clientDirTree = updateRequest->dir_tree();

    for (auto &clientPair : clientDirTree.directories())
    {
        // Dir on client but not on server
        if (serverDirTree->directories().find(clientPair.first) == serverDirTree->directories().end())
        {
            console->debug("New folder on server: {}", clientPair.second.relative_path());
            (*uploads->mutable_directories())[clientPair.second.relative_path()] = clientPair.second;
        }
    }

    for (auto &serverPair : serverDirTree->directories())
    {
        // Dir on server but not on client
        if (clientDirTree.directories().find(serverPair.first) == clientDirTree.directories().end())
        {
            console->debug("Folder missing on client {}: {}", updateRequest->client_id(), serverPair.second.relative_path());
            (*downloads->mutable_directories())[serverPair.second.relative_path()] = serverPair.second;
        }
    }

    updateResponse->set_allocated_uploads(uploads);
    updateResponse->set_allocated_downloads(downloads);
}

grpc::Status Server::Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                            DiSy::UpdateResponse *updateResponse)
{
    if (!context)
    {
        return grpc::Status::CANCELLED;
    }

    if (checkClientApproved(updateRequest))
    {
        processUpdate(updateRequest, updateResponse);
        return grpc::Status::OK;
    }
    else
    {
        return grpc::Status::CANCELLED;
    }
}

grpc::Status Server::GetDirectory(grpc::ServerContext *context, const DiSy::DirectoryMetadata *directoryMetadata,
                                  DiSy::Directory *directory)
{
    if (!context)
    {
        return grpc::Status::CANCELLED;
    }

    DiSy::DirectoryMetadata mData = *directoryMetadata;
    *directory = reader::getDirectoryFromMetadata(mData);

    return grpc::Status::OK;
}

grpc::Status Server::SendDirectory(grpc::ServerContext *context, const DiSy::Directory *directory,
                                   DiSy::Empty *empty)
{
    if (!context || !empty)
    {
        return grpc::Status::CANCELLED;
    }

    writer::writeDirectory(path, directory);

    return grpc::Status::OK;
}