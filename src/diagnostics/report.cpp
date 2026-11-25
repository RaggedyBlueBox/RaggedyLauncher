#include "raggedy/diagnostics/report.hpp"

#include <regex>

namespace raggedy::diagnostics
{
    namespace
    {
        const std::string kRedacted = "[redacted]";

        struct PatternSet
        {
            std::vector<std::regex> values;
            std::vector<std::regex> suspicious;
        };

        const PatternSet& patterns()
        {
            static const PatternSet set{
                {
                    std::regex{R"((token|secret|password|passwd|pin|authorization|refresh[_-]?token|access[_-]?token|account[_-]?id|xuid|api[_-]?key)\s*[:=]\s*("[^"]*"|'[^']*'|[^\s,;]+))", std::regex::icase},
                    std::regex{R"(\bbearer\s+[A-Za-z0-9._~+/=-]+)", std::regex::icase},
                    std::regex{R"([A-Za-z0-9_-]{8,}\.[A-Za-z0-9_-]{8,}\.[A-Za-z0-9_-]{8,})"},
                    std::regex{R"((https?|ftp|wss?)://[^\s"'<>]+)", std::regex::icase},
                    std::regex{R"([A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,})"},
                    std::regex{R"(([A-Za-z]:\\|\\\\)[^\s"'<>]+)"},
                    std::regex{R"((/(?:[A-Za-z][A-Za-z0-9._-]*/)+[A-Za-z0-9._-]*))"},
                    std::regex{R"((~/(?:[A-Za-z0-9._-]+/)*[A-Za-z0-9._-]+))"},
                    std::regex{R"(\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b)"},
                    std::regex{R"((?:::1|::ffff:[0-9.]+))"},
                    std::regex{R"((?=[0-9A-Fa-f:]*:[0-9A-Fa-f:]*:)[0-9A-Fa-f:]{3,39})"},
                    std::regex{R"(\b[A-Za-z]{2,5}-[A-Z0-9]{2,8}(?:-[A-Z0-9]{2,8})?\b)", std::regex::icase},
                    std::regex{R"(0[xX][0-9A-Fa-f]{4,})"},
                    std::regex{R"(\b[0-9A-Fa-f]{12,}\b)"},
                    std::regex{R"(\b[0-9]{8,}\b)"},
                    std::regex{R"re((?=[A-Za-z0-9_-]*[0-9])(?=[A-Za-z0-9_-]*[A-Za-z])[A-Za-z0-9_-]{24,})re"}
                },
                {
                    std::regex{R"((token|secret|password|passwd|pin|authorization|refresh[_-]?token|access[_-]?token|account[_-]?id|xuid|api[_-]?key)\s*[:=]\s*\S+)", std::regex::icase},
                    std::regex{R"(\bbearer\s+[A-Za-z0-9._~+/=-]+)", std::regex::icase},
                    std::regex{R"([A-Za-z0-9_-]{8,}\.[A-Za-z0-9_-]{8,}\.[A-Za-z0-9_-]{8,})"},
                    std::regex{R"((https?|ftp|wss?)://[^\s"'<>]+)", std::regex::icase},
                    std::regex{R"([A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,})"},
                    std::regex{R"(([A-Za-z]:\\|\\\\)[^\s"'<>]+)"},
                    std::regex{R"((/(?:[A-Za-z][A-Za-z0-9._-]*/)+[A-Za-z0-9._-]*))"},
                    std::regex{R"((~/(?:[A-Za-z0-9._-]+/)*[A-Za-z0-9._-]+))"},
                    std::regex{R"(\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b)"},
                    std::regex{R"((?:::1|::ffff:[0-9.]+))"},
                    std::regex{R"((?=[0-9A-Fa-f:]*:[0-9A-Fa-f:]*:)[0-9A-Fa-f:]{3,39})"},
                    std::regex{R"(\b[A-Za-z]{2,5}-[A-Z0-9]{2,8}(?:-[A-Z0-9]{2,8})?\b)", std::regex::icase},
                    std::regex{R"(0[xX][0-9A-Fa-f]{4,})"},
                    std::regex{R"(\b[0-9A-Fa-f]{12,}\b)"},
                    std::regex{R"(\b[0-9]{8,}\b)"},
                    std::regex{R"re((?=[A-Za-z0-9_-]*[0-9])(?=[A-Za-z0-9_-]*[A-Za-z])[A-Za-z0-9_-]{24,})re"}
                }
            };
            return set;
        }

        std::string single_line(std::string_view text)
        {
            std::string result(text);
            for (char& c : result)
            {
                if (c == '\n' || c == '\r' || c == '\t')
                {
                    c = ' ';
                }
            }
            return result;
        }
    }

    std::string redact_message(std::string_view message)
    {
        std::string result(message);
        for (const std::regex& pattern : patterns().values)
        {
            result = std::regex_replace(result, pattern, kRedacted);
        }
        return result;
    }

    DiagnosticReport make_report(std::string build_id, bool telemetry_opt_in, std::vector<DiagnosticEvent> events)
    {
        DiagnosticReport report;
        report.build_id = redact_message(build_id);
        report.telemetry_opt_in = telemetry_opt_in;
        report.events.reserve(events.size());
        for (const DiagnosticEvent& event : events)
        {
            report.events.push_back({redact_message(event.level), redact_message(event.message)});
        }
        return report;
    }

    std::string render_report(const DiagnosticReport& report)
    {
        std::string rendered = "Raggedy diagnostics\n";
        rendered += "Build: " + (report.build_id.empty() ? std::string{"unknown"} : single_line(report.build_id)) + "\n";
        rendered += std::string{"Telemetry: "} + (report.telemetry_opt_in ? "on" : "off") + "\n";
        rendered += "Events: " + std::to_string(report.events.size()) + "\n";
        for (const DiagnosticEvent& event : report.events)
        {
            rendered += "[" + single_line(event.level) + "] " + single_line(event.message) + "\n";
        }
        return rendered;
    }

    bool safe_to_send(const DiagnosticReport& report)
    {
        if (!report.telemetry_opt_in)
        {
            return false;
        }

        const std::string rendered = render_report(report);
        for (const std::regex& pattern : patterns().suspicious)
        {
            if (std::regex_search(rendered, pattern))
            {
                return false;
            }
        }
        return true;
    }
}
