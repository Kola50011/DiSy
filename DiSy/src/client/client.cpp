#include "client/client.hpp"
#include "shared/crawler.hpp"
#include "shared.hpp"

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
        console->info("updated success");
    }
    else
    {
        console->error("update error");
    }

    console->debug("Uploads: {}", updateResponse.uploads().DebugString());
    console->debug("Downloads: {}", updateResponse.downloads().DebugString());

    uploadDirectories(updateResponse.uploads().directories());
    downloadDirectories(updateResponse.downloads().directories());

    uploadFiles(updateResponse.uploads().files());
}

int64_t Client::getId()
{
    grpc::ClientContext clientContext;
    DiSy::Empty empty;
    DiSy::GetNewIdResponse getNewIdResponse;
    grpc::Status status{stub->GetNewId(&clientContext, empty, &getNewIdResponse)};

    if (status.ok())
    {
        console->info("GetNewId success");
    }
    else
    {
        console->error("GetNewId error: " + status.error_message());
    }
    return getNewIdResponse.id();
}

Client::Client(string _path, string grpcAddress, string _asioAddress, int _asioPort)
{
    auto channel{grpc::CreateChannel(grpcAddress, grpc::InsecureChannelCredentials())};
    stub = DiSy::Server::NewStub(channel);
    clientId = getId();
    path = _path;
    asioAddress = _asioAddress;
    asioPort = _asioPort;
}

Client::~Client() {}