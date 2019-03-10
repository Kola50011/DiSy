#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <grpcpp/grpcpp.h>
#include <vector>
#include <string>
#include <thread>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

class Server final : public DiSy::Server::Service
{
private:
  std::string path;
  int nextClientId{0};
  DiSy::DirTree *serverDirTree;
  std::thread asioThread;

  std::shared_ptr<spdlog::logger>
      console{spdlog::stderr_color_mt("server_logger")};

  void startAsioServer(short unsigned int port);

public:
  void updateServerTree(std::string path);
  bool checkClientApproved(const DiSy::UpdateRequest *updateRequest);
  void processUpdate(const DiSy::UpdateRequest *updateRequest, DiSy::UpdateResponse *updateResponse);

  grpc::Status GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                        DiSy::GetNewIdResponse *getNewIdResponse) override;
  grpc::Status Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                      DiSy::UpdateResponse *updateResponse) override;

  grpc::Status GetDirectory(grpc::ServerContext *context, const DiSy::DirectoryMetadata *directoryMetadata,
                            DiSy::Directory *directory) override;
  grpc::Status SendDirectory(grpc::ServerContext *context, const DiSy::Directory *directory,
                             DiSy::Empty *empty) override;

  Server(std::string path, int port);
  ~Server();
};