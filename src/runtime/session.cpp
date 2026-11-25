#include "raggedy/runtime/session.hpp"

#include <string>
#include <utility>

namespace raggedy::runtime
{
    GameSession::GameSession(platform::IProcessRunner& runner)
        : runner_(runner)
    {
    }

    bool GameSession::start(const SessionRequest& request, std::string& error)
    {
        error.clear();
        output_.clear();

        if (!request.authenticated || !request.entitlement_verified)
        {
            state_ = SessionState::failed;
            error = "Sign-in and entitlement verification are required before launch";
            return false;
        }

        state_ = SessionState::preparing;
        state_ = SessionState::launching;

        std::string process_output;
        const int code = runner_.run(request.plan.arguments, process_output);
        output_ = std::move(process_output);

        if (code == 0)
        {
            state_ = SessionState::exited;
            return true;
        }

        state_ = SessionState::failed;
        error = "Game process exited with code " + std::to_string(code);
        return false;
    }

    SessionState GameSession::state() const
    {
        return state_;
    }

    const std::string& GameSession::output() const
    {
        return output_;
    }
}
