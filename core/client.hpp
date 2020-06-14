#ifndef DISCORD_CLIENT_HPP
#define DISCORD_CLIENT_HPP

#include "config.hpp"

#include <string>
#include <memory>
#include <thread>
#include <limits>

// IXWebSocket forward declarations
namespace ix
{
    class WebSocket;
    class HttpClient;
    struct WebSocketMessage;
    using WebSocketMessagePtr = std::unique_ptr<WebSocketMessage>;
}

DISCORD_NS_BEGIN

struct Gateway;
struct Payload;

class Client
{
public:
    Client(const std::string &token);
    ~Client();

    /**
     * Gateway Opcodes
     * https://discord.com/developers/docs/topics/opcodes-and-status-codes
     */
    enum class GatewayOpcode : std::uint32_t
    {
        // Name                 Code        Client Action       Description
        DISPATCH                = 0,        // Receive          An event was dispatched.
        HEARTBEAT               = 1,        // Send/Receive     Fired periodically by the client to keep the connection alive.
        IDENTIFY                = 2,        // Send             Starts a new session during the initial handshake.
        PRESENCE_UPDATE         = 3,        // Send             Update the client's presence.
        VOICE_STATE_UPDATE      = 4,        // Send             Used to join/leave or move between voice channels.
        RESUME                  = 6,        // Send             Resume a previous session that was disconnected.
        RECONNECT               = 7,        // Receive          You should attempt to reconnect and resume immediately.
        REQUEST_GUILD_MEMBERS   = 8,        // Send             Request information about offline guild members in a large guild.
        INVALID_SESSION         = 9,        // Receive          The session has been invalidated. You should reconnect and identify/resume accordingly.
        HELLO                   = 10,       // Receive          Sent immediately after connecting, contains the heartbeat_interval to use.
        HEARTBEAT_ACK           = 11,       // Receive          Sent in response to receiving a heartbeat to acknowledge that it has been received.

        // gateway opcode could not be determined
        INVALID                 = 0xFFFF,
    };

    /**
     * Gateway Close Event Codes
     * https://discord.com/developers/docs/topics/opcodes-and-status-codes
     */
    enum class GatewayCloseEventCode
    {
        UNKNOWN_ERROR           = 4000, // We're not sure what went wrong. Try reconnecting?
        UNKNONW_OPCODE          = 4001, // You sent an invalid Gateway opcode or an invalid payload for an opcode. Don't do that!
        DECODE_ERROR            = 4002, // You sent an invalid payload to us. Don't do that!
        NOT_AUTHENTICATED       = 4003, // You sent us a payload prior to identifying.
        AUTHENTICATION_FAILED   = 4004, // The account token sent with your identify payload is incorrect.
        ALREADY_AUTHENTICATED   = 4005, // You sent more than one identify payload. Don't do that!
        INVALID_SEQ             = 4007, // The sequence sent when resuming the session was invalid. Reconnect and start a new session.
        RATE_LIMITED            = 4008, // Woah nelly! You're sending payloads to us too quickly. Slow it down! You will be disconnected on receiving this.
        SESSION_TIMED_OUT       = 4009, // Your session timed out. Reconnect and start a new one.
        INVALID_SHARD           = 4010, // You sent us an invalid shard when identifying.
        SHARDING_REQUIRED       = 4011, // The session would have handled too many guilds - you are required to shard your connection in order to connect.
        INVALID_API_VERSION     = 4012, // You sent an invalid version for the gateway.
        INVALID_INTENT          = 4013, // You sent an invalid intent for a Gateway Intent. You may have incorrectly calculated the bitwise value.
        DISALLOWED_INTENT       = 4014, // You sent a disallowed intent for a Gateway Intent. You may have tried to specify an intent that you have not enabled or are not whitelisted for.
    };

    /**
     * Gateway Intents
     * https://discord.com/developers/docs/topics/gateway#gateway-intents
     */
    enum class Intent : std::uint32_t
    {
        GUILDS = (1 << 0),
        //  - GUILD_CREATE
        //  - GUILD_UPDATE
        //  - GUILD_DELETE
        //  - GUILD_ROLE_CREATE
        //  - GUILD_ROLE_UPDATE
        //  - GUILD_ROLE_DELETE
        //  - CHANNEL_CREATE
        //  - CHANNEL_UPDATE
        //  - CHANNEL_DELETE
        //  - CHANNEL_PINS_UPDATE

        GUILD_MEMBERS = (1 << 1),
        //  - GUILD_MEMBER_ADD
        //  - GUILD_MEMBER_UPDATE
        //  - GUILD_MEMBER_REMOVE

        GUILD_BANS = (1 << 2),
        //  - GUILD_BAN_ADD
        //  - GUILD_BAN_REMOVE

        GUILD_EMOJIS = (1 << 3),
        //  - GUILD_EMOJIS_UPDATE

        GUILD_INTEGRATIONS = (1 << 4),
        //  - GUILD_INTEGRATIONS_UPDATE

        GUILD_WEBHOOKS = (1 << 5),
        //  - WEBHOOKS_UPDATE

        GUILD_INVITES = (1 << 6),
        //  - INVITE_CREATE
        //  - INVITE_DELETE

        GUILD_VOICE_STATES = (1 << 7),
        //  - VOICE_STATE_UPDATE

        GUILD_PRESENCES = (1 << 8),
        //  - PRESENCE_UPDATE

        GUILD_MESSAGES = (1 << 9),
        //  - MESSAGE_CREATE
        //  - MESSAGE_UPDATE
        //  - MESSAGE_DELETE
        //  - MESSAGE_DELETE_BULK

        GUILD_MESSAGE_REACTIONS = (1 << 10),
        //  - MESSAGE_REACTION_ADD
        //  - MESSAGE_REACTION_REMOVE
        //  - MESSAGE_REACTION_REMOVE_ALL
        //  - MESSAGE_REACTION_REMOVE_EMOJI

        GUILD_MESSAGE_TYPING = (1 << 11),
        //  - TYPING_START

        DIRECT_MESSAGES = (1 << 12),
        //  - CHANNEL_CREATE
        //  - MESSAGE_CREATE
        //  - MESSAGE_UPDATE
        //  - MESSAGE_DELETE
        //  - CHANNEL_PINS_UPDATE

        DIRECT_MESSAGE_REACTIONS = (1 << 13),
        //  - MESSAGE_REACTION_ADD
        //  - MESSAGE_REACTION_REMOVE
        //  - MESSAGE_REACTION_REMOVE_ALL
        //  - MESSAGE_REACTION_REMOVE_EMOJI

        DIRECT_MESSAGE_TYPING = (1 << 14),
        //  - TYPING_START

        // all guild intents
        ALL_GUILDS = GUILDS |
                     GUILD_MEMBERS |
                     GUILD_BANS |
                     GUILD_EMOJIS |
                     GUILD_INTEGRATIONS |
                     GUILD_WEBHOOKS |
                     GUILD_INVITES |
                     GUILD_VOICE_STATES |
                     GUILD_PRESENCES |
                     GUILD_MESSAGES |
                     GUILD_MESSAGE_REACTIONS |
                     GUILD_MESSAGE_TYPING,

        // all direct message intents
        ALL_DIRECT_MESSAGES = DIRECT_MESSAGES |
                              DIRECT_MESSAGE_REACTIONS |
                              DIRECT_MESSAGE_TYPING,

        // everything
        ALL = ALL_GUILDS | ALL_DIRECT_MESSAGES,

        // basic intents for basic command bots
        // some intents require a setting to be enabled in the Discord developer console
        DEFAULTS = GUILDS | GUILD_MESSAGES | DIRECT_MESSAGES | GUILD_MEMBERS,
    };

    constexpr inline void setIntents(Intent intents)
    {
        this->_intents = intents;
    }

    /**
     * Starts the Discord event loop.
     * This function is blocking and only returns on errors or on user shutdown.
     */
    int exec();

    /**
     * Stops the bot.
     */
    void stop();

private:
    int _ret = 0;

    bool _running = false;
    bool _terminate = false;

    std::string _token;
    std::shared_ptr<ix::WebSocket> _ws;
    std::shared_ptr<ix::HttpClient> _http;
    std::shared_ptr<Gateway> _gateway;

    std::uint32_t _heartbeat_interval = 0;
    std::int32_t _last_seq = -1;
    bool _heartbeat_ack_received = false;
    std::thread _heartbeat_thr;

    Intent _intents = Intent::DEFAULTS;
    std::string _session_id;

    void connect();
    void heartbeat();
    void stop_threads();

    void on_websocket_event(const ix::WebSocketMessagePtr &msg);
    void on_websocket_message(const ix::WebSocketMessagePtr &msg);

    const Payload parse_payload(const std::string &payload);

    void send_message(GatewayOpcode op, const std::string &message, bool log = true);
    void send_identity();
    void send_resume();
};

DISCORD_NS_END

constexpr inline Discord::Client::Intent operator | (Discord::Client::Intent lhs, Discord::Client::Intent rhs)
{
    return static_cast<Discord::Client::Intent>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

#endif // DISCORD_CLIENT_HPP
