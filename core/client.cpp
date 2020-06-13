#include "client.hpp"

#include <string_view>

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXHttpClient.h>

DISCORD_NS_BEGIN

namespace
{

// Discord API base URL
static const std::string_view URL("https://discordapp.com/api");

} // anonymous namespace

Client::Client(const std::string &token)
    : _token(token)
{
    this->_ws = std::make_shared<ix::WebSocket>();
    this->_http = std::make_shared<ix::HttpClient>();
}

Client::~Client()
{
}

DISCORD_NS_END
