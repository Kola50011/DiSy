#include "client/client.hpp"
#include "shared/crawler.hpp"
#include "shared/shared.hpp"
#include <asio.hpp>

#include "DiSy.grpc.pb.h"
#include "DiSy.pb.h"

using namespace std;

void Client::sendUpdate()
{
    grpc::ClientContext clientContext;
    DiSy::UpdateRequest updateRequest;
    updateRequest.set_client_id(clientId);
    updateRequest.set_allocated_dir_tree(crawler::crawlDirectory(path));
    updateRequest.set_time(shared::getCurrentTime());

    DiSy::UpdateResponse updateResponse;
    grpc::Status status{stub->Update(&clientContext, updateRequest, &updateResponse)};

    if (status.ok())
    {
        console->debug("Update success");
    }
    else
    {
        console->error("Update error: " + status.error_message());
        console->error("Retrying in 5s");
        this_thread::sleep_for(5s);
        return sendUpdate();
    }

    console->debug("Uploads: {}", updateResponse.uploads().DebugString());
    console->debug("Downloads: {}", updateResponse.downloads().DebugString());

    try
    {
        uploadDirectories(updateResponse.uploads().directories());
        downloadDirectories(updateResponse.downloads().directories());

        uploadFiles(updateResponse.uploads().files());
        downloadFiles(updateResponse.downloads().files());
    }
    catch (std::exception &e)
    {
        console->error("Error in communication with server! {}", e.what());
    }
}

int64_t Client::getId()
{
    grpc::ClientContext clientContext;
    DiSy::Empty empty;
    DiSy::GetNewIdResponse getNewIdResponse;
    grpc::Status status{stub->GetNewId(&clientContext, empty, &getNewIdResponse)};

    if (status.ok())
    {
        console->debug("GetNewId success");
    }
    else
    {
        console->error("GetNewId error: " + status.error_message());
        console->error("Retrying in 5s");
        this_thread::sleep_for(5s);
        return getId();
    }
    return getNewIdResponse.id();
}

Client::Client(string _path, string grpcAddress, string _asioAddress, int _asioPort)
{
    console->info("Client Started: "
                  "\n\tPath: {}"
                  "\n\tgRPC address: {}"
                  "\n\tasio address: {}:{}",
                  _path, grpcAddress, _asioAddress, _asioPort);

    auto channel{grpc::CreateChannel(grpcAddress, grpc::InsecureChannelCredentials())};
    stub = DiSy::Server::NewStub(channel);
    clientId = getId();
    path = _path;
    asioAddress = _asioAddress;
    asioPort = _asioPort;
}

Client::~Client() {}