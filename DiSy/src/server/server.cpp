#include <grpcpp/grpcpp.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

#include "server/server.hpp"

#include <iostream>

using namespace std;

grpc::Status Server::Connect(grpc::ServerContext *context, const DiSy::ConnectionRequest *connectionRequest,
                             const DiSy::ConnectionResponse *connectionResponse)
{
    cout << context->peer() << endl;
    cout << connectionRequest->time() << endl;

    connectionResponse->set_successful(true);

    return grpc::Status::OK;
}