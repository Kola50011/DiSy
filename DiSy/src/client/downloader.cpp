#include "client/client.hpp"
#include "shared/reader.hpp"
#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"
#include "shared/asioNetworking.hpp"
#include "shared/writer.hpp"

#include <asio.hpp>
#include <string>

using namespace std;
using namespace asio::ip;

using asio::buffer;
using asio::error_code;
using asio::streambuf;
using asio::write;
using asio::ip::address;
using asio::ip::tcp;
using std::istream;

void Client::downloadDirectories(google::protobuf::Map<string, DiSy::DirectoryMetadata> directories)
{
    for (auto &pair : directories)
    {
        downloadDirectory(pair.second);
    }
}

void Client::downloadDirectory(DiSy::DirectoryMetadata &directoryMetadata)
{
    grpc::ClientContext clientContext;
    DiSy::Directory directory;
    grpc::Status status{stub->GetDirectory(&clientContext, directoryMetadata, &directory)};

    const DiSy::Directory *directoryPointer = &directory;
    writer::writeDirectory(path, directoryPointer);

    if (status.ok())
    {
        console->debug("DownloadDirectory success {}", directoryMetadata.relative_path());
    }
    else
    {
        console->error("DownloadDirectory error: {}", status.error_message());
    }
}

void Client::downloadFiles(google::protobuf::Map<string, DiSy::FileMetadata> files)
{
    for (auto &pair : files)
    {
        downloadFile(pair.second);
    }
}

void Client::downloadFile(DiSy::FileMetadata &fileMetadata)
{
    asio::io_context ioContext;

    tcp::resolver resolver{ioContext};
    auto results = resolver.resolve(asioAddress, to_string(asioPort));
    tcp::socket socket{ioContext};

    asio::connect(socket, results);

    networking::sendProto(socket, fileMetadata);

    networking::MessageType messageType;

    networking::receiveProtoMessageType(socket, messageType);

    if (messageType != networking::MessageType::File)
    {
        console->error("Got invalid message type!");
        return;
    }

    DiSy::File file;
    networking::receiveProtoMessage(socket, file);

    socket.close();

    const DiSy::File *filePointer = &file;
    writer::writeFile(path, filePointer);

    console->debug("DownloadFile success {}", file.metadata().relative_path());
}