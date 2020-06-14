#ifndef DISCORD_CHANNEL_HPP
#define DISCORD_CHANNEL_HPP

#include "config.hpp"
#include "user.hpp"

#include <string>
#include <vector>

DISCORD_NS_BEGIN

/**
 * Discord Channel Type
 * https://discord.com/developers/docs/resources/channel#channel-object-channel-types
 */
enum class ChannelType
{
    GUILD_TEXT      = 0, // a text channel within a server
    DM              = 1, // a direct message between users
    GUILD_VOICE     = 2, // a voice channel within a server
    GROUP_DM        = 3, // a direct message between multiple users
    GUILD_CATEGORY  = 4, // an organizational category that contains up to 50 channels
    GUILD_NEWS      = 5, // a channel that users can follow and crosspost into their own server
    GUILD_STORE     = 6, // a channel in which game developers can sell their game on Discord
};

/**
 * Discord Channel Permission Overwrite
 * https://discord.com/developers/docs/resources/channel#overwrite-object
 */
struct ChannelPermissionOverwrite
{
    // TODO: add permission enum with | and & operators

    std::string id;     // role or user id
    std::string type;   // either "role" or "member"
    int allow;          // permission bit set
    int deny;           // permission bit set
};

/**
 * Discord Channel
 * https://discord.com/developers/docs/resources/channel#channel-object-channel-structure
 */
struct Channel
{
    std::string id;                     // the id of this channel
    ChannelType type;                   // the type of channel
    std::string guild_id;               // the id of the guild
    int position = -1;                  // sorting position of the channel
    std::vector<ChannelPermissionOverwrite> overwrites; // explicit permission overwrites for members and roles
    std::string name;                   // the name of the channel (2-100 characters)
    std::string topic;                  // the channel topic (0-1024 characters)
    bool nsfw = false;                  // whether the channel is nsfw
    std::string last_message_id;        // the id of the last message sent in this channel (may not point to an existing or valid message)
    int bitrate = -1;                   // the bitrate (in bits) of the voice channel
    int user_limit = -1;                // the user limit of the voice channel
    int rate_limit = -1;                // amount of seconds a user has to wait before sending another message (0-21600); bots, as well as users with the permission manage_messages or manage_channel, are unaffected
    std::vector<User> recipients;       // the recipients of the DM
    std::string icon;                   // icon hash
    std::string owner_id;               // id of the DM creator
    std::string app_id;                 // application id of the group DM creator if it is bot-created
    std::string parent_id;              // id of the parent category for a channel (each parent category can contain up to 50 channels)
    std::string last_pin_timestamp;     // when the last pinned message was pinned

    /**
     * Check if channel has an id.
     */
    constexpr inline operator bool() const
    {
        return !this->id.empty();
    }
};

DISCORD_NS_END

#endif // DISCORD_CHANNEL_HPP
