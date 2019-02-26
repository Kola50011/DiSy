#include "client/client.hpp"
#include "shared/reader.hpp"
#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"
#include "shared/asioNetworking.hpp"

#include <asio.hpp>
#include <string>

using namespace std;
using namespace asio::ip;

using asio::buffer;
using asio::streambuf;
using asio::write;
using asio::ip::address;
using asio::ip::tcp;
using std::istream;

void Client::uploadDirectories(google::protobuf::Map<string, DiSy::DirectoryMetadata> directories)
{
    for (auto &pair : directories)
    {
        uploadDirectory(pair.second);
    }
}

void Client::uploadDirectory(DiSy::DirectoryMetadata &directoryMetadata)
{
    grpc::ClientContext clientContext;
    DiSy::Directory directory = reader::getDirectoryFromMetadata(directoryMetadata);
    DiSy::Empty empty;
    grpc::Status status{stub->SendDirectory(&clientContext, directory, &empty)};

    if (status.ok())
    {
        console->debug("UploadDirectory success {}", directoryMetadata.relative_path());
    }
    else
    {
        console->error("UploadDirectory error: {}", status.error_message());
    }
    directory.release_metadata();
}

void Client::uploadFiles(google::protobuf::Map<string, DiSy::FileMetadata> files)
{
    for (auto &pair : files)
    {
        uploadFile(pair.second);
    }
}

void Client::uploadFile(DiSy::FileMetadata &fileMetadata)
{
    asio::io_context ioContext;

    tcp::resolver resolver{ioContext};
    auto results = resolver.resolve(asioAddress, to_string(asioPort));
    tcp::socket socket{ioContext};

    asio::connect(socket, results);

    DiSy::File file = reader::getFileFromMetadata(fileMetadata, path);
    networking::sendProto(socket, file);
    socket.close();

    file.release_metadata();
    console->debug("UploadFile success {}", fileMetadata.relative_path());
}