#include "raggedy/graphics/layer.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <utility>

namespace raggedy::graphics
{
    namespace
    {
        bool valid_extension_name(std::string_view value)
        {
            if (value.empty() || value.size() > 128)
            {
                return false;
            }

            for (const char raw : value)
            {
                const unsigned char c = static_cast<unsigned char>(raw);
                const bool allowed = std::isalnum(c) != 0
                    || c == '_' || c == '.' || c == '-' || c == '+';
                if (!allowed)
                {
                    return false;
                }
            }
            return true;
        }

        bool valid_relative_path(std::string_view value)
        {
            if (value.empty() || value.size() > 256 || value.front() == '/' || value.find('\\') != std::string_view::npos
                || value.find(':') != std::string_view::npos)
            {
                return false;
            }

            std::size_t start = 0;
            while (start <= value.size())
            {
                const std::size_t end = value.find('/', start);
                const std::size_t length = end == std::string_view::npos ? value.size() - start : end - start;
                const std::string_view part = value.substr(start, length);
                if (part.empty() || part == "." || part == "..")
                {
                    return false;
                }

                for (const char raw : part)
                {
                    const unsigned char c = static_cast<unsigned char>(raw);
                    const bool allowed = std::isalnum(c) != 0
                        || c == '_' || c == '.' || c == '-';
                    if (!allowed)
                    {
                        return false;
                    }
                }

                if (end == std::string_view::npos)
                {
                    break;
                }
                start = end + 1;
            }
            return true;
        }
    }

    bool valid_window_mode(WindowMode mode)
    {
        switch (mode)
        {
        case WindowMode::windowed:
        case WindowMode::borderless:
        case WindowMode::fullscreen:
            return true;
        }
        return false;
    }

    bool valid_window_config(const WindowConfig& config)
    {
        return config.width >= 640
            && config.width <= 7680
            && config.height >= 360
            && config.height <= 4320
            && valid_window_mode(config.mode)
            && !config.title.empty()
            && config.title.size() <= 128;
    }

    bool valid_context_config(const ContextConfig& config, std::string& error)
    {
        error.clear();
        if (!supported_translation(config.source, config.target))
        {
            error = "Unsupported graphics translation.";
            return false;
        }

        if (config.source == ApiFamily::opengl)
        {
            if (config.major < 1 || config.major > 4 || config.minor < 0 || config.minor > 9)
            {
                error = "OpenGL context version is invalid.";
                return false;
            }
        }
        else if (config.major != 1 || config.minor < 0 || config.minor > 3)
        {
            error = "Vulkan context version is invalid.";
            return false;
        }

        std::set<std::string> seen;
        for (const std::string& extension : config.extensions)
        {
            if (!valid_extension_name(extension))
            {
                error = "Graphics extension name is invalid.";
                return false;
            }
            if (!seen.insert(extension).second)
            {
                error = "Graphics extension is listed more than once.";
                return false;
            }
        }
        return true;
    }

    bool valid_resource_paths(const ResourcePaths& paths, std::string& error)
    {
        error.clear();
        if (!valid_relative_path(paths.root)
            || !valid_relative_path(paths.shaders)
            || !valid_relative_path(paths.cache)
            || !valid_relative_path(paths.diagnostics))
        {
            error = "Graphics resource paths must stay inside the app state directory.";
            return false;
        }
        return true;
    }

    std::vector<std::string> unsupported_extensions(const ContextConfig& config, const CapabilitySet& capabilities)
    {
        std::vector<std::string> result;
        for (const std::string& extension : config.extensions)
        {
            const auto found = std::find(capabilities.extensions.begin(), capabilities.extensions.end(), extension);
            if (found == capabilities.extensions.end())
            {
                result.push_back(extension);
            }
        }
        return result;
    }

    bool valid_capabilities(const CapabilitySet& capabilities, const ContextConfig& config, std::string& error)
    {
        std::string context_error;
        if (!valid_context_config(config, context_error))
        {
            error = std::move(context_error);
            return false;
        }
        if (!capabilities.can_present
            || capabilities.max_texture_size < 4096
            || capabilities.max_render_target_size < 4096)
        {
            error = "Graphics device does not meet the minimum capabilities.";
            return false;
        }
        if (!unsupported_extensions(config, capabilities).empty())
        {
            error = "Required graphics extensions are missing.";
            return false;
        }
        return true;
    }

    bool build_graphics_layer_plan(const WindowConfig& window,
                                   const ContextConfig& context,
                                   const ResourcePaths& paths,
                                   GraphicsLayerPlan& plan,
                                   std::string& error)
    {
        error.clear();
        if (!valid_window_config(window))
        {
            error = "Graphics window configuration is invalid.";
            return false;
        }
        if (!valid_context_config(context, error))
        {
            return false;
        }
        if (!valid_resource_paths(paths, error))
        {
            return false;
        }

        plan = GraphicsLayerPlan{};
        plan.window = window;
        plan.context = context;
        plan.paths = paths;
        plan.translation = translation_plan(context.source, context.target);
        plan.validation_steps = {
            "Window size and mode validated.",
            "Input routing uses the launcher navigation actions.",
            "Context request validated.",
            "Translation concepts enumerated.",
            "Resource paths kept relative to app state.",
            "Diagnostics require redaction and native validation."
        };
        return true;
    }

    std::vector<std::string> graphics_layer_report(const GraphicsLayerPlan& plan)
    {
        std::vector<std::string> report;
        report.push_back(std::string{"Source: "} + std::string{family_name(plan.context.source)});
        report.push_back(std::string{"Target: "} + std::string{family_name(plan.context.target)});
        report.push_back(std::string{"Window: "} + std::to_string(plan.window.width) + "x" + std::to_string(plan.window.height));
        report.push_back(std::string{"Translation notes: "} + std::to_string(plan.translation.size()));
        report.insert(report.end(), plan.validation_steps.begin(), plan.validation_steps.end());
        report.push_back("Native backend validation is still required.");
        return report;
    }
}
