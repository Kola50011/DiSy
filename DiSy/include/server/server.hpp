#pragma once

#include <grpcpp/grpcpp.h>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

class Server final : public DiSy::Server::Service
{
private:
  int nextClientId{0};
  DiSy::DirTree serverDirTree;

public:
  DiSy::DirTree getDifferenceAndSetState(DiSy::DirTree dirTree);

  grpc::Status Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                      DiSy::UpdateResponse *updateResponse) override;

  grpc::Status GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                        DiSy::GetNewIdResponse *getNewIdResponse) override;
};