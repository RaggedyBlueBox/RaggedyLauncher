#include "raggedy/controller/navigation.hpp"

#include <algorithm>

namespace raggedy::controller
{
    namespace
    {
        std::size_t clamped(std::size_t index, std::size_t count)
        {
            return count == 0 ? 0 : std::min(index, count - 1);
        }
    }

    NavigationResult apply_input(const NavigationState& state, InputAction action, bool repeat_phase, std::size_t pack_count, std::size_t row_count)
    {
        (void)repeat_phase;

        NavigationResult result;
        result.state = state;

        switch (action)
        {
        case InputAction::left:
            if (state.region == FocusRegion::pack_tabs)
            {
                result.state.pack_index = clamped(state.pack_index == 0 ? 0 : state.pack_index - 1, pack_count);
            }
            else if (state.region == FocusRegion::version_rows)
            {
                result.state.region = FocusRegion::pack_tabs;
                result.state.row_index = 0;
            }
            break;

        case InputAction::right:
            if (state.region == FocusRegion::pack_tabs)
            {
                const std::size_t next = state.pack_index >= pack_count ? 0 : state.pack_index + 1;
                result.state.pack_index = clamped(next, pack_count);
            }
            else if (state.region == FocusRegion::version_rows)
            {
                result.state.region = FocusRegion::pack_tabs;
                result.state.row_index = 0;
            }
            break;

        case InputAction::down:
            if (state.region == FocusRegion::pack_tabs)
            {
                result.state.region = FocusRegion::version_rows;
                result.state.row_index = clamped(state.row_index, row_count);
            }
            else
            {
                result.state.row_index = clamped(state.row_index >= row_count ? 0 : state.row_index + 1, row_count);
            }
            break;

        case InputAction::up:
            if (state.region != FocusRegion::pack_tabs)
            {
                result.state.row_index = state.row_index == 0 ? 0 : state.row_index - 1;
            }
            break;

        case InputAction::confirm:
            result.confirm = true;
            break;

        case InputAction::back:
            result.go_back = true;
            if (state.region == FocusRegion::action_rows || state.region == FocusRegion::settings_rows || state.region == FocusRegion::credits_rows)
            {
                result.state.region = FocusRegion::action_rows;
            }
            else
            {
                result.state.region = FocusRegion::pack_tabs;
            }
            result.state.row_index = 0;
            break;

        case InputAction::none:
            break;
        }

        result.changed = result.state.region != state.region
            || result.state.pack_index != state.pack_index
            || result.state.row_index != state.row_index;
        return result;
    }

    std::chrono::milliseconds repeat_delay()
    {
        return std::chrono::milliseconds{300};
    }

    std::chrono::milliseconds repeat_interval()
    {
        return std::chrono::milliseconds{100};
    }
}
