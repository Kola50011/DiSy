#include "spdlog/spdlog.h"
#include "CLI11.hpp"
#include <grpcpp/grpcpp.h>

#include "shared.hpp"
#include "client/crawler.hpp"
#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

using namespace std;

void eraseSubStr(std::string &mainStr, const std::string &toErase)
{
    size_t pos = mainStr.find(toErase);
    if (pos != string::npos)
    {
        mainStr.erase(pos, toErase.length());
    }
}

int64_t getId(shared_ptr<spdlog::logger> &console, unique_ptr<DiSy::Server::Stub> &stub)
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

void sendUpdate(shared_ptr<spdlog::logger> &console, unique_ptr<DiSy::Server::Stub> &stub, int64_t clientId, string path)
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
}

int main(int argc, char const *argv[])
{
    // init
    auto console = spdlog::stderr_color_mt("console");
    CLI ::App app{"DiSy client"};

    string path{"default"};
    string address{"0.0.0.0:8080"};
    app.add_option("-d,--dir", path, "Directory to synchronize")->required()->check(CLI::ExistingDirectory);
    app.add_option("-a,--addres", address, "Server address");
    CLI11_PARSE(app, argc, argv);

    // Main
    auto channel{grpc::CreateChannel(address, grpc::InsecureChannelCredentials())};
    auto stub = DiSy::Server::NewStub(channel);

    // Get client ID
    int64_t clientId = getId(console, stub);

    // Update
    while (true)
    {
        sendUpdate(console, stub, clientId, path);
        this_thread::sleep_for(5s);
    }
    return 0;
}
