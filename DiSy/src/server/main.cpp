#include "spdlog/spdlog.h"
#include "CLI11.hpp"
#include <grpcpp/grpcpp.h>

#include "server/server.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    auto console = spdlog::stderr_color_mt("console");
    CLI ::App app{"DiSy server"};

    string address{"0.0.0.0:8080"};
    app.add_option("-a,--addres", address, "Server address");
    CLI11_PARSE(app, argc, argv);

    Server server;
    grpc::ServerBuilder serverBuilder;
    serverBuilder.AddListeningPort(address, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&server);
    std::unique_ptr<grpc::Server> serverPtr(serverBuilder.BuildAndStart());

    console->info("Server running on " + address);
    serverPtr->Wait();

    return 0;
}
