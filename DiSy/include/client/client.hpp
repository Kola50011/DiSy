#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
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

  std::string asioAddress;
  int asioPort;

  int64_t getId();

  void uploadDirectories(google::protobuf::Map<std::string, DiSy::DirectoryMetadata> directories);
  void uploadDirectory(DiSy::DirectoryMetadata &directoryMetadata);

  void downloadDirectories(google::protobuf::Map<std::string, DiSy::DirectoryMetadata> directories);
  void downloadDirectory(DiSy::DirectoryMetadata &directoryMetadata);

  void uploadFiles(google::protobuf::Map<std::string, DiSy::FileMetadata> files);
  void uploadFile(DiSy::FileMetadata &fileMetadata);

  void downloadFiles(google::protobuf::Map<std::string, DiSy::FileMetadata> files);
  void downloadFile(DiSy::FileMetadata &fileMetadata);

public:
  Client(std::string _path, std::string grpcAddress, std::string _asioAddress, int _asioPort);
  ~Client();

  void sendUpdate();
};