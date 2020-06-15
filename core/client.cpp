#include "client.hpp"
#include "utils/os.hpp"

#include <stdexcept>
#include <functional>
#include <chrono>

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXHttpClient.h>

#include <nlohmann/json.hpp>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <magic_enum.hpp>

using json = nlohmann::json;

DISCORD_NS_BEGIN

namespace
{

// logging helper
template<typename... Args>
static constexpr inline void log(const std::string &fmt, Args&&... args)
{
#ifdef DEBUG_BUILD
    fmt::print("\033[1m[Client]\033[0m " + fmt + "\n", args...);
#else
    // TODO: file logging
    fmt::print("\033[1m[Client]\033[0m " + fmt + "\n", args...);
#endif
}

// minimal json value parsing helper
template<typename T>
static constexpr inline T get_json_value(const json &j, const std::string &key)
{
    try {
        return j[key].get<T>();
    } catch (...) {
        if constexpr (std::is_integral<T>::value)
        {
            return 0; // ensure on errors the number is 0 on all platforms
        }
        else
        {
            return T{}; // for non-integral types use the default constructed value
        }
    }
}

static inline std::int64_t current_time_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// Discord API base URL
static const std::string URL("https://discordapp.com/api");

// Discord API Endpoints
static const std::string URL_BOT_GATEWAY(URL + "/gateway/bot");
static const std::string URL_WSS_SUFFIX("/?v=6&encoding=json");

} // anonymous namespace

/**
 * Gateway Response
 */
struct Gateway
{
    std::string url;
    std::uint32_t shards;

    struct
    {
        std::uint32_t total;
        std::uint32_t remaining;
        std::uint32_t reset_after;
        std::uint32_t max_concurrency;
    } limit;
};

/**
 * WebSocket Payload
 * https://discord.com/developers/docs/topics/gateway#payloads
 *
 * s and t are optimal and can be null
 */
struct Payload
{
    bool valid = false;
    Client::GatewayOpcode op;   // opcode number [op]
    json msg;                   // event data [d]
    std::uint32_t s;            // sequence number, used for resuming sessions and heartbeats [s]
    std::string t;              // the event name for this payload [t]

    /**
     * Serialize the payload object for sending.
     */
    const std::string serialize() const
    {
        json j;
        j["op"] = static_cast<std::uint32_t>(this->op);
        j["d"] = this->msg;
        return j.dump();
    }
};

Client::Client(const std::string &token)
    : _token(token)
{
    // check if the token is empty
    if (this->_token.empty())
    {
        throw std::invalid_argument("bot token is empty");
        return;
    }

    this->_ws = std::make_shared<ix::WebSocket>();
    this->_http = std::make_shared<ix::HttpClient>();

    // Note: on Windows this call is required, but since I don't support Windows
    // this function call is useless, if someone wants to tink with this library on Windows uncomment this line
    //ix::initNetSystem();

    auto args = ix::HttpRequestArgsPtr(new ix::HttpRequestArgs());
    args->extraHeaders["Authorization"] = "Bot " + this->_token;

    // request the gateway endpoint for bots
    auto res = this->_http->get(URL_BOT_GATEWAY, args);

    log("request({}) status={}", URL_BOT_GATEWAY, res->statusCode);

    if (res->statusCode == 200)
    {
        try {
            auto j = json::parse(res->payload);
            log("response: {}", j.dump());
            this->_gateway = std::make_shared<Gateway>();
            this->_gateway->url = j["url"].get<std::string>();
            this->_gateway->shards = j["shards"].get<std::uint32_t>();
            this->_gateway->limit = {
                j["session_start_limit"]["total"].get<std::uint32_t>(),
                j["session_start_limit"]["remaining"].get<std::uint32_t>(),
                j["session_start_limit"]["reset_after"].get<std::uint32_t>(),
                j["session_start_limit"]["max_concurrency"].get<std::uint32_t>(),
            };
        } catch (json::exception &e) {
            throw std::runtime_error(fmt::format("invalid JSON response received: {}", e.what()));
            return;
        }
    }
    else
    {
        log("error({}) status={}", res->errorMsg, res->statusCode);
        throw std::runtime_error("request failed");
        return;
    }
}

Client::~Client()
{
}

int Client::exec()
{
    // start the ws connection in a new thread
    this->connect();

    // wait until the bot quits
    while (this->_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    this->_ws->stop();

    // return with status code
    return this->_ret;
}

void Client::stop()
{
    this->_running = false;
    this->_terminate = true;

    this->stop_threads();
}

void Client::connect()
{
    // open websocket connection
    this->_ws->setUrl(this->_gateway->url + URL_WSS_SUFFIX);
    this->_ws->setOnMessageCallback(std::bind(&Client::on_websocket_event, this, std::placeholders::_1));
    this->_ws->start();
    this->_running = true;
    this->_terminate = false;
}

void Client::heartbeat()
{
    while (!this->_terminate)
    {
        // attempt a reconnect
        if (!this->_heartbeat_ack_received)
        {
            this->_ws->stop();
            this->stop_threads();
            this->connect();
        }

        this->send_message(GatewayOpcode::HEARTBEAT, this->_last_seq == -1 ? "" : std::to_string(this->_last_seq));
        this->_heartbeat_ack_received = false;

        const auto begin = current_time_ms();
        while (((current_time_ms() - begin) < this->_heartbeat_interval) && !this->_terminate)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    this->_heartbeat_cv.notify_all();
}

void Client::stop_threads()
{
    this->_terminate = true;

    // wait for all threads to exit
    std::unique_lock lk{this->_heartbeat_cv_mutex};
    this->_heartbeat_cv.wait(lk, [&]{ return this->_terminate; });

    // terminate heartbeat thread
    if (this->_heartbeat_thr.joinable())
    {
        this->_heartbeat_thr.join();
    }
}

void Client::on_websocket_event(const ix::WebSocketMessagePtr &msg)
{
    switch (msg->type)
    {
        // handle ws open event
        case ix::WebSocketMessageType::Open:
            this->_ret = 0;
            log("WebSocket connection opened: {} [{}]", this->_gateway->url + URL_WSS_SUFFIX, msg->openInfo.protocol);
            break;

        // handle ws error event
        case ix::WebSocketMessageType::Error:
            this->_ret = 1;
            log("WebSocket connection error: {}", msg->errorInfo.reason);
            break;

        // handle ws close event
        case ix::WebSocketMessageType::Close:
            log("WebSocket connection closed: {} [{}]", msg->closeInfo.reason, msg->closeInfo.code);

            // TODO: handle all the gateway errors

            if (msg->closeInfo.code == static_cast<std::uint16_t>(GatewayCloseEventCode::DISALLOWED_INTENT))
            {
                log("Disallowed intents, shutting down bot...");
                this->_ret = 1;
                this->stop();
            }
            else if (msg->closeInfo.code != ix::WebSocketCloseConstants::kNormalClosureCode)
            {
                log("Abnormal closure, attempting reconnect...");
                this->_ret = 1;
                this->_ws->stop();
                this->stop_threads();
                this->connect();
            }
            else
            {
                // shutdown bot on normal closure or user shutdown
                this->_ret = 0;
                this->stop();
            }
            break;

        // handle discord messages
        case ix::WebSocketMessageType::Message:
            this->_ret = 0;
            this->on_websocket_message(msg);
            break;
    }
}

void Client::on_websocket_message(const ix::WebSocketMessagePtr &msg)
{
    log("received message: {}", msg->str);

    auto payload = this->parse_payload(msg->str);
    log("parsed payload: {}", payload);
    if (!payload.valid)
    {
        return;
    }

    // initialize or send heartbeat
    if (payload.op == GatewayOpcode::HELLO)
    {
        this->_heartbeat_interval = get_json_value<decltype(this->_heartbeat_interval)>(payload.msg, "heartbeat_interval");
        if (this->_heartbeat_interval == 0)
        {
            log("failed to obtain the heartbeat interval, shutting down bot...");
            this->stop();
            return;
        }

        if (this->_session_id.empty())
        {
            this->send_identity();
        }
        else
        {
            this->send_resume();
        }

        this->_heartbeat_ack_received = true;
        this->_terminate = false;

        // terminate previous heartbeat thread if running
        if (this->_heartbeat_thr.joinable())
        {
            this->_heartbeat_thr.join();
        }

        // start new heartbeat thread
        this->_heartbeat_thr = std::thread(&Client::heartbeat, this);
    }

    // heartbeat acknowledged, keep session active
    else if (payload.op == GatewayOpcode::HEARTBEAT_ACK)
    {
        this->_heartbeat_ack_received = true;
    }

    // dispatch (most of the code goes here and is split up into multiple files and functions)
    else if (payload.op == GatewayOpcode::DISPATCH)
    {
        // store last sequence
        this->_last_seq = payload.s;

        // bot is ready, obtain some data for session restore
        if (payload.t == "READY")
        {
            this->_session_id = payload.msg["session_id"].get<std::string>();
        }

        // a guild was created, called for each guild the bot is a member of
        else if (payload.t == "GUILD_CREATE")
        {

        }

        // TODO: lets get started with the actual bot functionality
    }

    // session is invalid
    else if (payload.op == GatewayOpcode::INVALID_SESSION)
    {
        log("received a invalid session response");

        if (payload.msg == "true")
        {
            log("trying to resume session...");
            this->send_resume();
        }
        else
        {
            log("reconnecting with a new session...");
            this->_ws->stop();
            this->stop_threads();
            this->connect();
        }
    }
}

const Payload Client::parse_payload(const std::string &payload)
{
    try {
        auto j = json::parse(payload);
        Payload payload;
        payload.op = static_cast<GatewayOpcode>(j["op"].get<std::uint32_t>());
        payload.msg = j["d"]; // contains event data based on opcode
        payload.s = j.at("s").is_null() ? 0 : j["s"].get<std::uint32_t>();
        payload.t = j.at("t").is_null() ? "" : j["t"].get<std::string>();
        payload.valid = true;
        return payload;
    } catch (json::exception &e) {
        log("parsing the payload failed, ignoring message: {}", e.what());
    }

    return {};
}

void Client::send_message(GatewayOpcode op, const std::string &message, bool _log)
{
    if (_log)
    {
        log("sending message ({}): {}", magic_enum::enum_name(op), message);
    }
    else
    {
        log("sending message ({}): [CONTENTS REDACTED DUE TO SENSITIVE DATA]", magic_enum::enum_name(op));
    }

    Payload payload;
    payload.op = op;
    try {
        payload.msg = json::parse(message);
    } catch (json::exception &e) {
        payload.msg = message;
    }

    this->_ws->send(payload.serialize());
}

void Client::send_identity()
{
    json id;
    id["token"] = this->_token;
    id["properties"]["$os"] = Utils::get_os_name();
    id["properties"]["$browser"] = "misaka-oneesama";
    id["properties"]["$device"] = "misaka-oneesama";
    id["intents"] = static_cast<std::uint32_t>(this->_intents);

    this->send_message(GatewayOpcode::IDENTIFY, id.dump(), false);
}

void Client::send_resume()
{
    json resume;
    resume["token"] = this->_token;
    resume["session_id"] = this->_session_id;
    resume["seq"] = this->_last_seq;

    this->send_message(GatewayOpcode::RESUME, resume.dump(), false);
}

DISCORD_NS_END

template<>
struct fmt::formatter<Discord::Payload>
{
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const Discord::Payload &pl, FormatContext &ctx)
    {
        if (pl.valid)
        {
            return format_to(
                ctx.out(),
                "Payload{{op={} ({}), d={}, s={}, t=\"{}\"}}",
                magic_enum::enum_name(pl.op), pl.op, pl.msg.dump(), pl.s, pl.t);
        }
        else
        {
            return format_to(ctx.out(), "Payload{{invalid}}");
        }
    }
};
