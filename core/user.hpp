#ifndef DISCORD_USER_HPP
#define DISCORD_USER_HPP

#include "config.hpp"

#include <string>
#include <vector>
#include <cstdint>

DISCORD_NS_BEGIN

/**
 * Discord User Flags
 * https://discord.com/developers/docs/resources/user#user-object-user-flags
 */
enum class UserFlag : std::uint32_t
{
    NONE = 0,
    DISCORD_EMPLOYEE = (1 << 0),
    DISCORD_PARTNER = (1 << 1),
    HYPESQUAD_EVENTS = (1 << 2),
    BUG_HUNTER_LEVEL_1 = (1 << 3),
    HOUSE_BRAVERY = (1 << 6),
    HOUSE_BRILLIANCE = (1 << 7),
    HOUSE_BALANCE = (1 << 8),
    EARLY_SUPPORTER = (1 << 9),
    TEAM_USER = (1 << 10),
    SYSTEM = (1 << 12),
    BUG_HUNTER_LEVEL_2 = (1 << 14),
    VERIFIED_BOT = (1 << 16),
    VERIFIED_BOT_DEVELOPER = (1 << 17),
};

/**
 * Discord User Premium Type
 * https://discord.com/developers/docs/resources/user#user-object-premium-types
 */
enum class PremiumType
{
    NONE = 0,
    NITRO_CLASSIC = 1,
    NITRO = 2,
};

/**
 * Discord User Object
 * https://discord.com/developers/docs/resources/user#user-object
 */
struct User
{
    std::string id;                 // the user's id
    std::string username;           // the user's username, not unique across the platform
    std::string discriminator;      // the user's 4-digit discord-tag
    std::string avatar;             // the user's avatar hash
    bool bot = false;               // whether the user belongs to an OAuth2 application
    bool system = false;            // whether the user is an Official Discord System user (part of the urgent message system)
    bool mfa_enabled = false;       // whether the user has two factor enabled on their account
    std::string locale;             // the user's chosen language option
    bool verified = false;          // whether the email on this account has been verified
    std::string email;              // the user's email
    UserFlag flags;                 // the flags on a user's account
    PremiumType premium_type;       // the type of Nitro subscription on a user's account
    UserFlag public_flags;          // the public flags on a user's account
};

DISCORD_NS_END

constexpr inline Discord::UserFlag operator| (Discord::UserFlag lhs, Discord::UserFlag rhs)
{
    return static_cast<Discord::UserFlag>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

constexpr inline Discord::UserFlag operator& (Discord::UserFlag lhs, Discord::UserFlag rhs)
{
    return static_cast<Discord::UserFlag>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
}

#endif // DISCORD_USER_HPP
