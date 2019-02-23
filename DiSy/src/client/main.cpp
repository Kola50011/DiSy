#include "spdlog/spdlog.h"
#include "CLI11.hpp"

#include "client/client.hpp"
#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

using namespace std;

int main(int argc, char const *argv[])
{
    // init
    CLI ::App app{"DiSy client"};

    string path{"default"};
    string grpcAddress{"0.0.0.0:8080"};
    string asioAddress{"0.0.0.0"};
    int asioPort{8081};
    bool debug{false};
    app.add_option("-d,--dir", path, "Directory to synchronize")->required()->check(CLI::ExistingDirectory);
    app.add_option("-g,--gaddres", grpcAddress, "grpc server address");
    app.add_option("-a,--aaddres", asioAddress, "asio server address");
    app.add_option("-p,--port", asioPort, "asio server port");
    app.add_flag("--debug", debug, "debug messages");
    CLI11_PARSE(app, argc, argv);

    if (debug)
    {
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    }

    Client client = Client(path, grpcAddress, asioAddress, asioPort);

    // Update
    while (true)
    {
        client.sendUpdate();
        this_thread::sleep_for(5s);
    }
    return 0;
}
