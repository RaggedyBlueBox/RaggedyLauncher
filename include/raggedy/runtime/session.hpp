#pragma once

#include "raggedy/core/launch_plan.hpp"
#include "raggedy/platform/interfaces.hpp"

#include <string>

namespace raggedy::runtime
{
    enum class SessionState
    {
        idle,
        preparing,
        launching,
        running,
        exited,
        failed
    };

    struct SessionRequest
    {
        core::LaunchPlan plan;
        bool authenticated = false;
        bool entitlement_verified = false;
    };

    class GameSession
    {
    public:
        GameSession(platform::IProcessRunner& runner);
        bool start(const SessionRequest& request, std::string& error);
        SessionState state() const;
        const std::string& output() const;

    private:
        platform::IProcessRunner& runner_;
        SessionState state_ = SessionState::idle;
        std::string output_;
    };
}
