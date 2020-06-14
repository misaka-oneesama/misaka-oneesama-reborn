#ifndef DISCORD_MESSAGE_HPP
#define DISCORD_MESSAGE_HPP

#include "config.hpp"
#include "user.hpp"

#include <string>

DISCORD_NS_BEGIN

/**
 * Message Embed Object
 * https://discord.com/developers/docs/resources/channel#embed-object-embed-structure
 */
struct Embed
{
    std::string title;          // title of embed
    std::string type;           // type of embed (always "rich" for webhook embeds)
    std::string description;    // description of embed
    std::string url;            // description of embed

    // TODO: incomplete

    /**
     * A valid embed requires a title and description.
     */
    constexpr inline operator bool() const
    {
        return !(this->title.empty() && this->description.empty());
    }
};

/**
 * Discord Message Object
 * https://discord.com/developers/docs/resources/channel#message-object
 */
struct Message
{
    std::string id;                 // id of the message
    std::string channel_id;         // id of the channel the message was sent in
    std::string guild_id;           // id of the guild the message was sent in
    User author;                    // the author of this message (not guaranteed to be a valid user)
    // member                       // member properties for this message's author
    std::string content;            // contents of the message
    std::string timestamp;          // when this message was sent
    std::string edited_timestamp;   // when this message was edited (or null if never)
    bool tts = false;               // whether this was a TTS message
    bool mention_everyone = false;  // whether this message mentions everyone

    // TODO: incomplete
};

DISCORD_NS_END

#endif // DISCORD_MESSAGE_HPP
