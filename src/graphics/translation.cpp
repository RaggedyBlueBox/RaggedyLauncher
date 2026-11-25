#include "raggedy/graphics/translation.hpp"

#include <array>

namespace raggedy::graphics
{
    bool supported_translation(ApiFamily source, ApiFamily target)
    {
        const bool source_ok = source == ApiFamily::opengl || source == ApiFamily::vulkan;
        const bool target_ok = target == ApiFamily::direct3d11 || target == ApiFamily::direct3d12;
        return source_ok && target_ok;
    }

    std::vector<TranslationNote> translation_plan(ApiFamily source, ApiFamily target)
    {
        if (!supported_translation(source, target))
        {
            return {};
        }

        const std::array<ConceptKind, 8> concepts{
            ConceptKind::surface,
            ConceptKind::command_buffer,
            ConceptKind::pipeline_state,
            ConceptKind::texture_view,
            ConceptKind::sampler,
            ConceptKind::descriptor_binding,
            ConceptKind::shader_module,
            ConceptKind::render_pass
        };

        std::vector<TranslationNote> result;
        result.reserve(concepts.size());
        for (const ConceptKind item : concepts)
        {
            result.push_back(TranslationNote{
                item,
                source,
                target,
                item == ConceptKind::descriptor_binding || item == ConceptKind::render_pass
            });
        }
        return result;
    }

    std::string_view family_name(ApiFamily family)
    {
        switch (family)
        {
        case ApiFamily::opengl:
            return "OpenGL";
        case ApiFamily::vulkan:
            return "Vulkan";
        case ApiFamily::direct3d11:
            return "Direct3D 11";
        case ApiFamily::direct3d12:
            return "Direct3D 12";
        }
        return "Unknown";
    }
}
