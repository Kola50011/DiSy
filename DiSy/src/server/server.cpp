#include <grpcpp/grpcpp.h>
#include <iostream>
#include <math.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"
#include "server/server.hpp"
#include "shared.hpp"

using namespace std;

bool areEqual(DiSy::DirectoryMetadata &firstDirectory, DiSy::DirectoryMetadata &secondDirectory)
{
    if (firstDirectory.relative_path() == secondDirectory.relative_path())
    {
        return true;
    }
    return false;
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

    vector<DiSy::DirectoryMetadata> directoryRequests;
    auto clientDirTree = updateRequest->dir_tree();

    for (auto &clientPair : clientDirTree.directories())
    {
        // Dir on client but not on server
        if (serverDirTree.directories().find(clientPair.first) == serverDirTree.directories().end())
        {
            console->debug("New folder on server: {}", clientPair.second.relative_path());
            (*serverDirTree.mutable_directories())[clientPair.second.relative_path()] = clientPair.second;
        }
    }

    for (auto &serverPair : serverDirTree.directories())
    {
        // Dir on server but not on client
        if (clientDirTree.directories().find(serverPair.first) == clientDirTree.directories().end())
        {
            console->debug("Folder missing on client {}: {}", updateRequest->client_id(), serverPair.second.relative_path());
            directoryRequests.push_back(serverPair.second);
        }
    }
}

bool Server::isDirectoryRequested(DiSy::DirectoryMetadata directoryToCheck)
{
    for (auto &requestPair : directoryRequests)
    {
        for (auto directoyMetadata : requestPair.second)
        {
            if (areEqual(directoryToCheck, directoyMetadata))
            {
                return true;
            }
        }
    }
    return false;
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