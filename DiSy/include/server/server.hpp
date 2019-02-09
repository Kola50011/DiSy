#pragma once
#include "spdlog/spdlog.h"
#include <grpcpp/grpcpp.h>
#include <vector>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

class Server final : public DiSy::Server::Service
{
private:
  int nextClientId{0};
  DiSy::DirTree serverDirTree;

  std::map<int64_t, std::vector<DiSy::DirectoryMetadata>> directoryRequests;
  std::map<int64_t, std::vector<DiSy::FileMetadata>> fileRequests;

  std::shared_ptr<spdlog::logger>
      console{spdlog::stderr_color_mt("server_logger")};

public:
  bool checkClientApproved(const DiSy::UpdateRequest *updateRequest);
  void processUpdate(const DiSy::UpdateRequest *updateRequest, DiSy::UpdateResponse *updateResponse);
  bool isDirectoryRequested(DiSy::DirectoryMetadata);
  bool isFileRequested(DiSy::FileMetadata);

  grpc::Status Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                      DiSy::UpdateResponse *updateResponse) override;
  grpc::Status GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                        DiSy::GetNewIdResponse *getNewIdResponse) override;
};