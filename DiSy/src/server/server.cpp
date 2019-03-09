#include <grpcpp/grpcpp.h>
#include <iostream>
#include <math.h>
#include <asio.hpp>

#include "shared/asioNetworking.hpp"
#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"
#include "server/server.hpp"
#include "shared/shared.hpp"
#include "shared/crawler.hpp"
#include "shared/writer.hpp"
#include "shared/reader.hpp"

using namespace std;
using namespace asio::ip;
using asio::error_code;

Server::Server(string _path, int port)
{
    serverDirTree = crawler::crawlDirectory(_path);
    path = _path;

    asioThread = thread(&Server::startAsioServer, this, port);
}

Server::~Server()
{
    asioThread.join();
}

void Server::startAsioServer(short unsigned int port)
{
    asio::io_context ioContext;
    tcp::endpoint endpoint{tcp::v4(), port};
    tcp::acceptor acceptor{ioContext, endpoint};
    acceptor.listen();

    while (true)
    {
        error_code errorCode;

        tcp::socket socket{acceptor.accept(errorCode)};

        if (errorCode)
        {
            console->error("Error in socket: {}", errorCode.message());
            socket.close();
            continue;
        }

        networking::MessageType messageType;
        networking::receiveProtoMessageType(socket, messageType);

        if (messageType == networking::MessageType::File)
        {
            DiSy::File file;
            networking::receiveProtoMessage(socket, file);
            console->debug("Receive file {}", file.metadata().relative_path());
            (*serverDirTree->mutable_files())[file.metadata().relative_path()] = file.metadata();

            const DiSy::File *filePointer = &file;
            writer::writeFile(path, filePointer);
        }
        else if (messageType == networking::MessageType::FileMetadata)
        {
            DiSy::FileMetadata fileMetadata;
            networking::receiveProtoMessage(socket, fileMetadata);
            console->debug("Receive metadata {}", fileMetadata.relative_path());

            auto file = reader::getFileFromMetadata(fileMetadata, path);
            networking::sendProto(socket, file);
            file.release_metadata();
        }
        else
        {
            console->debug("Receive invalid message type");
        }

        socket.close();
    }
}

bool areEqual(DiSy::DirectoryMetadata &firstDirectory, DiSy::DirectoryMetadata &secondDirectory)
{
    if (firstDirectory.relative_path() == secondDirectory.relative_path())
    {
        return true;
    }
    return false;
}

grpc::Status Server::GetNewId(grpc::ServerContext *context, const DiSy::Empty *empty,
                              DiSy::GetNewIdResponse *getNewIdResponse)
{
    if (!context || !empty)
    {
        return grpc::Status::CANCELLED;
    }
    getNewIdResponse->set_id(nextClientId);
    nextClientId += 1;
    return grpc::Status::OK;
}

bool Server::checkClientApproved(const DiSy::UpdateRequest *updateRequest)
{
    uint32_t currentServerTime = shared::getCurrentTime();
    uint32_t currentClientTime = updateRequest->time();

    if (max(currentServerTime, currentClientTime) - min(currentServerTime, currentClientTime) < 1000)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Server::processUpdate(const DiSy::UpdateRequest *updateRequest,
                           DiSy::UpdateResponse *updateResponse)
{
    if (!updateResponse)
    {
        return;
    }

    DiSy::DirTree *uploads = new DiSy::DirTree();
    DiSy::DirTree *downloads = new DiSy::DirTree();

    auto clientDirTree = updateRequest->dir_tree();

    for (auto &clientPair : clientDirTree.directories())
    {
        // Dir on client but not on server
        if (serverDirTree->directories().find(clientPair.first) == serverDirTree->directories().end())
        {
            console->debug("New folder on server: {}", clientPair.second.relative_path());
            (*uploads->mutable_directories())[clientPair.second.relative_path()] = clientPair.second;
        }
    }

    for (auto &serverPair : serverDirTree->directories())
    {
        // Dir on server but not on client
        if (clientDirTree.directories().find(serverPair.first) == clientDirTree.directories().end())
        {
            console->debug("Folder missing on client {}: {}", updateRequest->client_id(), serverPair.second.relative_path());
            (*downloads->mutable_directories())[serverPair.second.relative_path()] = serverPair.second;
        }
    }

    for (auto &clientPair : clientDirTree.files())
    {
        // File on client but not on server
        if (serverDirTree->files().find(clientPair.first) == serverDirTree->files().end())
        {
            console->debug("New file on server: {}", clientPair.second.relative_path());
            (*uploads->mutable_files())[clientPair.second.relative_path()] = clientPair.second;
        }
        else
        {
            // Check timestamps & hash
            auto serverFileMetadata = serverDirTree->files().at(clientPair.first);
            auto clientFileMetadata = clientPair.second;

            if (serverFileMetadata.last_modified_time() < clientFileMetadata.last_modified_time() &&
                serverFileMetadata.hash() != clientFileMetadata.hash())
            {
                // Server has older file than client
                (*uploads->mutable_files())[clientPair.second.relative_path()] = clientPair.second;
            }
        }
    }

    for (auto &serverPair : serverDirTree->files())
    {
        // Dir on server but not on client
        if (clientDirTree.files().find(serverPair.first) == clientDirTree.files().end())
        {
            console->debug("File missing on client {}: {}", updateRequest->client_id(), serverPair.second.relative_path());
            (*downloads->mutable_files())[serverPair.second.relative_path()] = serverPair.second;
        }
        else
        {
            // Check timestamps & hash
            auto clientFileMetadata = clientDirTree.files().at(serverPair.first);
            auto serverFileMetadata = serverPair.second;

            if (serverFileMetadata.last_modified_time() > clientFileMetadata.last_modified_time() &&
                serverFileMetadata.hash() != clientFileMetadata.hash())
            {
                // Server has newer file than client
                (*downloads->mutable_files())[serverPair.second.relative_path()] = serverPair.second;
            }
        }
    }

    updateResponse->set_allocated_uploads(uploads);
    updateResponse->set_allocated_downloads(downloads);
}

grpc::Status Server::Update(grpc::ServerContext *context, const DiSy::UpdateRequest *updateRequest,
                            DiSy::UpdateResponse *updateResponse)
{
    if (!context)
    {
        return grpc::Status::CANCELLED;
    }

    if (checkClientApproved(updateRequest))
    {
        processUpdate(updateRequest, updateResponse);
        return grpc::Status::OK;
    }
    else
    {
        return grpc::Status::CANCELLED;
    }
}

grpc::Status Server::GetDirectory(grpc::ServerContext *context, const DiSy::DirectoryMetadata *directoryMetadata,
                                  DiSy::Directory *directory)
{
    if (!context)
    {
        return grpc::Status::CANCELLED;
    }

    DiSy::DirectoryMetadata mData = *directoryMetadata;
    *directory = reader::getDirectoryFromMetadata(mData);
    directory->release_metadata();
    return grpc::Status::OK;
}

grpc::Status Server::SendDirectory(grpc::ServerContext *context, const DiSy::Directory *directory,
                                   DiSy::Empty *empty)
{
    if (!context || !empty)
    {
        return grpc::Status::CANCELLED;
    }

    writer::writeDirectory(path, directory);
    (*serverDirTree->mutable_directories())[directory->metadata().relative_path()] = directory->metadata();

    return grpc::Status::OK;
}