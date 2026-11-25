#pragma once

#include <chrono>
#include <cstddef>

namespace raggedy::controller
{
    enum class FocusRegion
    {
        pack_tabs,
        version_rows,
        action_rows,
        settings_rows,
        credits_rows
    };

    enum class InputAction
    {
        none,
        up,
        down,
        left,
        right,
        confirm,
        back
    };

    struct NavigationState
    {
        FocusRegion region = FocusRegion::pack_tabs;
        std::size_t pack_index = 0;
        std::size_t row_index = 0;
    };

    struct NavigationResult
    {
        NavigationState state;
        bool changed = false;
        bool confirm = false;
        bool go_back = false;
    };

    NavigationResult apply_input(const NavigationState& state, InputAction action, bool repeat_phase, std::size_t pack_count, std::size_t row_count);

    std::chrono::milliseconds repeat_delay();
    std::chrono::milliseconds repeat_interval();
}
