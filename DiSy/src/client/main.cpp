#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "CLI11.hpp"
#include "json.hpp"
#include <iostream>

#include "client/client.hpp"
#include "DiSy.pb.h"
#include "DiSy.grpc.pb.h"

#include "shared/crawler.hpp"

using json = nlohmann::json;
using namespace std;

int main(int argc, char const *argv[])
{
    // init
    CLI ::App app{"DiSy client"};

    string path{"default"};
    string grpcAddress{"0.0.0.0:8080"};
    string asioAddress{"0.0.0.0"};
    string config{"config.json"};
    int asioPort{8081};
    bool debug{false};
    app.add_option("-d,--dir", path, "Directory to synchronize")->check(CLI::ExistingDirectory);
    app.add_option("-c,--config", config, "Config file");
    app.add_option("-g,--gaddres", grpcAddress, "grpc server address");
    app.add_option("-a,--aaddres", asioAddress, "asio server address");
    app.add_option("-p,--port", asioPort, "asio server port");
    app.add_flag("--debug", debug, "debug messages");
    CLI11_PARSE(app, argc, argv);

    if (config != "")
    {
        ifstream configFile(config);

        if (configFile.good())
        {
            json j;
            configFile >> j;

            path = j.value("path", path);
            grpcAddress = j.value("grpcAddress", grpcAddress);
            asioAddress = j.value("asioAddress", asioAddress);
            asioPort = j.value("asioPort", asioPort);
            debug = j.value("debug", debug);
            cout << "Config applied" << endl;
        }
        else
        {
            json j;
            j["path"] = path;
            j["grpcAddress"] = grpcAddress;
            j["asioAddress"] = asioAddress;
            j["asioPort"] = asioPort;
            j["debug"] = debug;

            std::ofstream outfile(config);
            outfile << j << std::endl;
            outfile.close();
        }
    }

    if (debug)
    {
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    }

    if (!crawler::pathExists(path))
    {
        cerr << "Path " << path << " invalid!" << std::endl;
        return -1;
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
