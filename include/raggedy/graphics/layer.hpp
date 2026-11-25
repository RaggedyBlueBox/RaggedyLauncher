#pragma once

#include "raggedy/graphics/translation.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::graphics
{
    enum class WindowMode
    {
        windowed,
        borderless,
        fullscreen
    };

    struct WindowConfig
    {
        int width = 1280;
        int height = 720;
        WindowMode mode = WindowMode::windowed;
        std::string title = "Raggedy Launcher";
    };

    struct ContextConfig
    {
        ApiFamily source = ApiFamily::opengl;
        ApiFamily target = ApiFamily::direct3d11;
        int major = 4;
        int minor = 5;
        bool debug = false;
        std::vector<std::string> extensions;
    };

    struct ResourcePaths
    {
        std::string root = "state/graphics";
        std::string shaders = "shaders";
        std::string cache = "cache";
        std::string diagnostics = "diagnostics";
    };

    struct CapabilitySet
    {
        int max_texture_size = 0;
        int max_render_target_size = 0;
        bool can_present = false;
        std::vector<std::string> extensions;
    };

    struct GraphicsLayerPlan
    {
        WindowConfig window;
        ContextConfig context;
        ResourcePaths paths;
        std::vector<TranslationNote> translation;
        std::vector<std::string> validation_steps;
    };

    bool valid_window_mode(WindowMode mode);
    bool valid_window_config(const WindowConfig& config);
    bool valid_context_config(const ContextConfig& config, std::string& error);
    bool valid_resource_paths(const ResourcePaths& paths, std::string& error);
    std::vector<std::string> unsupported_extensions(const ContextConfig& config, const CapabilitySet& capabilities);
    bool valid_capabilities(const CapabilitySet& capabilities, const ContextConfig& config, std::string& error);
    bool build_graphics_layer_plan(const WindowConfig& window,
                                   const ContextConfig& context,
                                   const ResourcePaths& paths,
                                   GraphicsLayerPlan& plan,
                                   std::string& error);
    std::vector<std::string> graphics_layer_report(const GraphicsLayerPlan& plan);
}
