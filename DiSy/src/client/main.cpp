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
    string address{"0.0.0.0:8080"};
    bool debug{false};
    app.add_option("-d,--dir", path, "Directory to synchronize")->required()->check(CLI::ExistingDirectory);
    app.add_option("-a,--addres", address, "Server address");
    app.add_flag("--debug", debug, "debug messages");
    CLI11_PARSE(app, argc, argv);

    if (debug)
    {
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    }

    Client client = Client(path, address);

    // Update
    while (true)
    {
        client.sendUpdate();
        this_thread::sleep_for(5s);
    }
    return 0;
}
