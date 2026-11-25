#include "raggedy/remote/session.hpp"

#include <algorithm>
#include <cctype>

namespace raggedy::remote
{
    namespace
    {
        constexpr std::chrono::minutes kSessionLifetime{15};

        bool valid_port(int port)
        {
            return port > 0 && port <= 65535;
        }

        bool valid_octet(std::string_view value)
        {
            if (value.empty() || value.size() > 3 || (value.size() > 1 && value.front() == '0'))
            {
                return false;
            }

            int number = 0;
            for (const char c : value)
            {
                if (c < '0' || c > '9')
                {
                    return false;
                }
                number = number * 10 + (c - '0');
            }
            return number <= 255;
        }

        bool dotted_quad(std::string_view host, int first, int second_low, int second_high)
        {
            int parts[4]{};
            std::size_t count = 0;
            std::size_t begin = 0;
            while (count < 4)
            {
                const std::size_t dot = host.find('.', begin);
                const std::string_view part = host.substr(begin, dot == std::string_view::npos ? std::string_view::npos : dot - begin);
                if (!valid_octet(part))
                {
                    return false;
                }

                int number = 0;
                for (const char c : part)
                {
                    number = number * 10 + (c - '0');
                }
                parts[count++] = number;
                if (dot == std::string_view::npos)
                {
                    break;
                }
                begin = dot + 1;
            }
            return count == 4
                && parts[0] == first
                && parts[1] >= second_low
                && parts[1] <= second_high;
        }

        bool local_ipv4(std::string_view host)
        {
            if (host.size() < 4 || host.find_first_not_of("0123456789.") != std::string_view::npos)
            {
                return false;
            }

            const std::size_t first_dot = host.find('.');
            if (first_dot == std::string_view::npos)
            {
                return false;
            }

            const std::string_view prefix = host.substr(0, first_dot);
            if (!valid_octet(prefix))
            {
                return false;
            }

            if (prefix == "10")
            {
                return dotted_quad(host, 10, 0, 255);
            }
            if (prefix == "172")
            {
                return dotted_quad(host, 172, 16, 31);
            }
            if (prefix == "192")
            {
                const std::size_t second_dot = host.find('.', first_dot + 1);
                if (second_dot == std::string_view::npos || host.substr(first_dot + 1, second_dot - first_dot - 1) != "168")
                {
                    return false;
                }
                return dotted_quad(host, 192, 168, 168);
            }
            return false;
        }
    }

    bool valid_pin(std::string_view pin)
    {
        if (pin.size() < 6 || pin.size() > 12)
        {
            return false;
        }
        return std::all_of(pin.begin(), pin.end(), [](char c)
        {
            return c >= '0' && c <= '9';
        });
    }

    RemoteSession create_session(std::string_view pin, int port, std::chrono::steady_clock::time_point now)
    {
        RemoteSession session;
        if (valid_pin(pin))
        {
            session.pin = std::string(pin);
        }
        session.created = now;
        session.port = port;
        session.active = valid_pin(session.pin) && valid_port(port);
        return session;
    }

    bool session_active(const RemoteSession& session, std::chrono::steady_clock::time_point now, std::chrono::seconds lifetime)
    {
        if (!session.active || !valid_pin(session.pin) || !valid_port(session.port) || lifetime.count() <= 0 || now < session.created)
        {
            return false;
        }
        return now - session.created < lifetime;
    }

    bool allowed_lan_host(std::string_view host)
    {
        if (host.empty())
        {
            return false;
        }

        std::string value(host);
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });

        if (value == "localhost" || value == "127.0.0.1" || value == "::1" || value == "[::1]")
        {
            return true;
        }

        if (value.size() > 6 && value.compare(value.size() - 6, 6, ".local") == 0)
        {
            const std::string prefix = value.substr(0, value.size() - 6);
            const bool valid_prefix = std::all_of(prefix.begin(), prefix.end(), [](unsigned char c)
            {
                return std::isalnum(c) != 0 || c == '-' || c == '.';
            });
            if (valid_prefix && prefix.front() != '-' && prefix.back() != '-' && prefix.front() != '.')
            {
                return true;
            }
        }

        return local_ipv4(value);
    }

    bool allowed_operation(const RemoteSession& session, RemoteOperationType operation, std::chrono::steady_clock::time_point now)
    {
        switch (operation)
        {
        case RemoteOperationType::list_files:
        case RemoteOperationType::read_file:
        case RemoteOperationType::write_file:
        case RemoteOperationType::upload:
        case RemoteOperationType::download:
            break;
        default:
            return false;
        }
        return session_active(session, now, std::chrono::duration_cast<std::chrono::seconds>(kSessionLifetime));
    }

    std::string session_status(const RemoteSession& session, std::chrono::steady_clock::time_point now, std::chrono::seconds lifetime)
    {
        if (!session.active || !valid_pin(session.pin))
        {
            return "Remote tools are off.";
        }
        if (!session_active(session, now, lifetime))
        {
            return "Remote session expired. Start a new session.";
        }
        return "Remote tools are on. LAN access only.";
    }
}
