#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>

#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

class Client
{
private:
  std::shared_ptr<spdlog::logger>
      console{spdlog::stderr_color_mt("client_logger")};
  std::shared_ptr<DiSy::Server::Stub> stub;

  int64_t clientId;
  std::string path;

  int64_t getId();

  void uploadDirectories(google::protobuf::Map<std::string, DiSy::DirectoryMetadata> directories);
  void uploadDirectory(DiSy::DirectoryMetadata &directoryMetadata);

public:
  Client(std::string path, std::string address);
  ~Client();

  void sendUpdate();
};