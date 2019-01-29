#pragma once

#include <grpcpp/grpcpp.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

class Server final : public DiSy::Server::Service
{
  public:
    grpc::Status Connect(grpc::ServerContext *context, const DiSy::ConnectionRequest *connectionRequest,
                         const DiSy::ConnectionResponse *connectionResponse) override;
};