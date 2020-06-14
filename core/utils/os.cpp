#include "os.hpp"

#include <sys/utsname.h>

const std::string Utils::get_os_name()
{
    struct utsname utsname;

    if (uname(&utsname) == 0)
    {
        return std::string(utsname.sysname);
    }

    return "unknown";
}
