#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::diagnostics
{
    struct DiagnosticEvent
    {
        std::string level;
        std::string message;
    };

    struct DiagnosticReport
    {
        std::string build_id;
        bool telemetry_opt_in = false;
        std::vector<DiagnosticEvent> events;
    };

    std::string redact_message(std::string_view message);

    DiagnosticReport make_report(std::string build_id, bool telemetry_opt_in, std::vector<DiagnosticEvent> events);

    std::string render_report(const DiagnosticReport& report);

    bool safe_to_send(const DiagnosticReport& report);
}
