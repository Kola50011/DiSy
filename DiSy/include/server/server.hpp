#pragma once

#include <grpcpp/grpcpp.h>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

class Server final : public DiSy::Server::Service
{
private:
  DiSy::DirTree currentDirTree;

public:
  DiSy::DirTree getDifferenceAndSetState(DiSy::DirTree dirTree);

  grpc::Status Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                      DiSy::UpdateResponse *updateResponse) override;
};