// THIS IS CURRENTLY A MINIMAL TEST APPLICATION

#include <fstream>
#include <vector>
#include <csignal>

#include <client.hpp>

#include <fmt/printf.h>

std::unique_ptr<Discord::Client> client;

void terminate(int sig)
{
    if (client)
    {
        client->stop();
    }

    // restore default signal handler to force quit on subsequent signals
    // when the bot deadlocked
    std::signal(sig, SIG_DFL);
}

int main(int argc, char **argv)
{
    std::signal(SIGINT, &terminate);
    std::signal(SIGTERM, &terminate);
    std::signal(SIGQUIT, &terminate);
    std::signal(SIGABRT, &terminate);

    // ignore hangups and keep bot running
    std::signal(SIGHUP, SIG_IGN);

    std::ifstream ifs("./token", std::ios_base::in | std::ios_base::ate);
    std::string token;
    if (ifs.is_open())
    {
        const auto size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        // read entire file
        std::vector<char> buffer(size);
        ifs.read(&buffer[0], size);
        ifs.close();

        // convert to std::string
        token = std::string(buffer.data(), buffer.size());
    }

    try {
        client = std::make_unique<Discord::Client>(token);
        return client->exec();
    } catch (std::exception &e) {
        fmt::print("{}\n", e.what());
        return 50;
    }
}
