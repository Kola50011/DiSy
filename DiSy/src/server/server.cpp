#include <grpcpp/grpcpp.h>
#include <iostream>
#include <math.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"
#include "server/server.hpp"
#include "shared.hpp"

using namespace std;

DiSy::DirTree Server::getDifferenceAndSetState(DiSy::DirTree dirTree)
{
    for (auto &pair : dirTree.directories())
    {
        cout << pair.first << endl;
    }
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