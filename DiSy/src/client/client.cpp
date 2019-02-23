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
    updateRequest.set_client_id(1);
    updateRequest.set_allocated_dir_tree(crawler::crawlDirectory(path));
    updateRequest.set_time(shared::getCurrentTime());

    console->debug("Sending to server: {}", updateRequest.DebugString());
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
}

Client::Client(string _path, string address)
{
    auto channel{grpc::CreateChannel(address, grpc::InsecureChannelCredentials())};
    stub = DiSy::Server::NewStub(channel);
    path = _path;
}

Client::~Client() {}