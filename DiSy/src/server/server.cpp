#include <grpcpp/grpcpp.h>
#include <iostream>
#include <math.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"
#include "server/server.hpp"
#include "shared.hpp"

using namespace std;

DiSy::DirTree Server::getDifferenceAndSetState(DiSy::DirTree clientDirTree)
{
    vector<DiSy::DirectoryMetadata> directoryRequests;
    vector<DiSy::DirectoryMetadata> directorySends;

    for (auto &clientPair : clientDirTree.directories())
    {
        // Dir on client but not on server
        if (serverDirTree.directories().find(clientPair.first) == serverDirTree.directories().end())
        {
            (*serverDirTree.mutable_directories())[clientPair.second.relative_path()] = clientPair.second;
        }
    }

    for (auto &serverPair : serverDirTree.directories())
    {
        // Dir on server but not on client
        if (clientDirTree.directories().find(serverPair.first) == clientDirTree.directories().end())
        {
            directoryRequests.push_back(serverPair.second);
        }
    }

    cout << serverDirTree.DebugString() << endl;

    return DiSy::DirTree{};
}

bool checkClientApproved(const DiSy::UpdateRequest *updateRequest)
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

grpc::Status Server::Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                            DiSy::UpdateResponse *updateResponse)
{
    cout << context->peer() << endl;
    // cout << updateRequest->DebugString() << endl;
    cout << updateResponse->DebugString() << endl;
    cout << getDifferenceAndSetState(updateRequest->dir_tree()).DebugString() << endl;

    if (checkClientApproved(updateRequest))
    {
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