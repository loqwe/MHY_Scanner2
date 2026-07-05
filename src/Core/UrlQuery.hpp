#pragma once

#include <string>
#include <string_view>

[[nodiscard]] inline std::string getUrlQueryParam(const std::string_view url, const std::string_view key)
{
    const auto queryPos = url.find('?');
    if (queryPos == std::string_view::npos || queryPos + 1 >= url.size())
    {
        return {};
    }

    std::string_view query = url.substr(queryPos + 1);
    while (!query.empty())
    {
        const auto nextPos = query.find('&');
        const std::string_view item = query.substr(0, nextPos);
        const auto eqPos = item.find('=');
        const std::string_view name = item.substr(0, eqPos);

        if (name == key)
        {
            if (eqPos == std::string_view::npos)
            {
                return {};
            }
            return std::string(item.substr(eqPos + 1));
        }

        if (nextPos == std::string_view::npos)
        {
            break;
        }
        query.remove_prefix(nextPos + 1);
    }

    return {};
}
