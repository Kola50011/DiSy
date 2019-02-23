#include "spdlog/spdlog.h"
#include "CLI11.hpp"
#include <grpcpp/grpcpp.h>

#include "server/server.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    auto console = spdlog::stderr_color_mt("console");
    CLI ::App app{"DiSy server"};

    string grpcAddress{"0.0.0.0:8080"};
    int asioPort{8081};
    string path{"default"};
    bool debug{false};
    app.add_option("-a,--addres", grpcAddress, "grpc address");
    app.add_option("-p,--port", asioPort, "asio port");
    app.add_option("-d,--dir", path, "Directory to synchronize")->required()->check(CLI::ExistingDirectory);
    app.add_flag("--debug", debug, "debug messages");

    CLI11_PARSE(app, argc, argv);

    if (debug)
    {
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    }

    Server server = Server(path, asioPort);

    grpc::ServerBuilder serverBuilder;
    serverBuilder.AddListeningPort(grpcAddress, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&server);
    std::unique_ptr<grpc::Server> serverPtr(serverBuilder.BuildAndStart());

    console->info("Server running on grpc: " + grpcAddress + " asio: " + std::to_string(asioPort));
    serverPtr->Wait();

    return 0;
}
