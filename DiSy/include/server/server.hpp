#pragma once

#include "spdlog/spdlog.h"
#include <grpcpp/grpcpp.h>
#include <vector>
#include <string>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

class Server final : public DiSy::Server::Service
{
private:
  std::string path;
  int nextClientId{0};
  DiSy::DirTree *serverDirTree;

  std::shared_ptr<spdlog::logger>
      console{spdlog::stderr_color_mt("server_logger")};

public:
  void updateServerTree(std::string path);
  bool checkClientApproved(const DiSy::UpdateRequest *updateRequest);
  void processUpdate(const DiSy::UpdateRequest *updateRequest, DiSy::UpdateResponse *updateResponse);

  grpc::Status GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                        DiSy::GetNewIdResponse *getNewIdResponse) override;
  grpc::Status Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                      DiSy::UpdateResponse *updateResponse) override;
  grpc::Status SendDirectory(grpc::ServerContext *context, const DiSy::Directory *directory,
                             DiSy::Empty *empty) override;

  Server(std::string path);
};