#pragma once

namespace raggedy::ui
{
    struct Rgba
    {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 1.0f;
    };

    struct Theme
    {
        Rgba background;
        Rgba panel;
        Rgba panel_alt;
        Rgba text;
        Rgba muted;
        Rgba accent_blue;
        Rgba accent_mint;
        Rgba focus;
        Rgba danger;
        Rgba success;
        float corner_radius = 18.0f;
        float title_size = 40.0f;
        float heading_size = 26.0f;
        float body_size = 18.0f;
    };

    constexpr Rgba kBackground{0.043f, 0.063f, 0.129f, 1.0f};
    constexpr Rgba kPanel{0.078f, 0.106f, 0.212f, 1.0f};
    constexpr Rgba kPanelAlt{0.106f, 0.141f, 0.267f, 1.0f};
    constexpr Rgba kText{0.949f, 0.965f, 1.0f, 1.0f};
    constexpr Rgba kMuted{0.659f, 0.714f, 0.847f, 1.0f};
    constexpr Rgba kAccentBlue{0.243f, 0.522f, 1.0f, 1.0f};
    constexpr Rgba kAccentMint{0.310f, 0.886f, 0.757f, 1.0f};
    constexpr Rgba kFocus{1.0f, 0.804f, 0.322f, 1.0f};
    constexpr Rgba kDanger{1.0f, 0.392f, 0.392f, 1.0f};
    constexpr Rgba kSuccess{0.310f, 0.886f, 0.573f, 1.0f};

    constexpr Theme kTheme{
        kBackground,
        kPanel,
        kPanelAlt,
        kText,
        kMuted,
        kAccentBlue,
        kAccentMint,
        kFocus,
        kDanger,
        kSuccess,
        18.0f,
        40.0f,
        26.0f,
        18.0f
    };
}
