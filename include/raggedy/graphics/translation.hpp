#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace raggedy::graphics
{
    enum class ApiFamily
    {
        opengl,
        vulkan,
        direct3d11,
        direct3d12
    };

    enum class ConceptKind
    {
        surface,
        command_buffer,
        pipeline_state,
        texture_view,
        sampler,
        descriptor_binding,
        shader_module,
        render_pass
    };

    struct TranslationNote
    {
        ConceptKind api_concept = ConceptKind::surface;
        ApiFamily source = ApiFamily::opengl;
        ApiFamily target = ApiFamily::direct3d12;
        bool needs_validation = true;
    };

    bool supported_translation(ApiFamily source, ApiFamily target);
    std::vector<TranslationNote> translation_plan(ApiFamily source, ApiFamily target);
    std::string_view family_name(ApiFamily family);
}
