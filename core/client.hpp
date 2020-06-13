#ifndef DISCORD_CLIENT_HPP
#define DISCORD_CLIENT_HPP

#include "config.hpp"

#include <string>
#include <memory>

namespace ix
{
    class WebSocket;
    class HttpClient;
}

DISCORD_NS_BEGIN

class Client
{
public:
    Client(const std::string &token);
    ~Client();

    /**
     * Gateway Opcodes
     * https://discord.com/developers/docs/topics/opcodes-and-status-codes
     */
    enum class GatewayOpcode
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

private:
    std::string _token;
    std::shared_ptr<ix::WebSocket> _ws;
    std::shared_ptr<ix::HttpClient> _http;
};

DISCORD_NS_END

#endif // DISCORD_CLIENT_HPP
