#include "renderer_policy.h"

#include "i18n.h"
#include "launcher_common.h"
#include "runtime_manager.h"

using i18n::Tr;
using i18n::TrFormat;

namespace {

std::wstring RendererPath(const std::wstring& runtimeRoot) {
    return runtimeRoot + L"\\runtime\\renderer.txt";
}

RendererBackend ParseRenderer(const std::wstring& text) {
    if (ToLowerW(TrimWhitespace(text)) == L"vulkan-experimental") return RendererBackend::VulkanExperimental;
    return RendererBackend::OpenGL;
}

bool FileExistsNonEmpty(const std::wstring& path) {
    WIN32_FILE_ATTRIBUTE_DATA data = {};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) return false;
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
    return data.nFileSizeHigh != 0 || data.nFileSizeLow != 0;
}

std::wstring StampOf(const std::wstring& path) {
    return FileStamp(path);
}

}  // namespace

RendererBackend RequestedRenderer(const std::wstring& runtimeRoot) {
    std::wstring text;
    if (!ReadTextFile(RendererPath(runtimeRoot), text)) return RendererBackend::OpenGL;
    return ParseRenderer(text);
}

const wchar_t* RendererBackendName(RendererBackend backend) {
    return backend == RendererBackend::VulkanExperimental ? L"Vulkan (experimental)" : L"OpenGL";
}

VulkanProbeResult ProbeVulkanBridge(const std::wstring& packageDir, const std::wstring& runtimeRoot) {
    VulkanProbeResult result;
    const std::wstring nativesDir = packageDir + L"\\natives";
    const std::wstring dozenPath = nativesDir + L"\\vulkan_dzn.dll";
    const std::wstring loaderPath = nativesDir + L"\\vulkan-1.dll";
    const std::wstring sdlPath = nativesDir + L"\\SDL3.dll";
    const bool hasDozen = FileExistsNonEmpty(dozenPath);
    const bool hasLoader = FileExistsNonEmpty(loaderPath);
    const bool hasSdl = FileExistsNonEmpty(sdlPath);
    std::wstring detail = L"vulkan_dzn.dll=" + std::wstring(hasDozen ? L"present" : L"missing");
    detail += L" vulkan-1.dll=" + std::wstring(hasLoader ? L"present" : L"missing");
    detail += L" SDL3.dll=" + std::wstring(hasSdl ? L"present" : L"missing");
    if (hasDozen) detail += L" dzn-stamp=" + StampOf(dozenPath);
    result.detail = detail;
    WriteLogF(L"Vulkan bridge probe: %s", detail.c_str());
    // A DLL being packaged is not a working bridge. End-to-end validation
    // (loader + Dozen ICD discovery, instance/device negotiation, SDL3
    // surface and presentation on Xbox UWP, shader stack, memory behavior)
    // has not passed on this console, so the bridge stays unavailable.
    // When that validation lands, it must set usable=true here only after
    // observing a real device, surface and present path.
    result.usable = false;
    if (!hasDozen) {
        result.reason = L"No Vulkan Dozen ICD is packaged (natives\\vulkan_dzn.dll is missing).";
    } else if (!hasLoader) {
        result.reason = L"No Vulkan loader is packaged (natives\\vulkan-1.dll is missing).";
    } else {
        result.reason = L"Vulkan artifacts are present but the bridge has not passed end-to-end validation on Xbox.";
    }
    (void)runtimeRoot;
    return result;
}

bool SetRequestedRenderer(
    const std::wstring& runtimeRoot, const std::wstring& packageDir,
    RendererBackend backend, std::wstring& note) {
    if (backend == RendererBackend::VulkanExperimental) {
        const VulkanProbeResult probe = ProbeVulkanBridge(packageDir, runtimeRoot);
        if (!probe.usable) {
            note = Tr(L"Vulkan is not usable yet: ") + probe.reason;
            WriteLogF(L"Vulkan request refused: %s", probe.reason.c_str());
            return false;
        }
    }
    const std::wstring value = backend == RendererBackend::VulkanExperimental
        ? L"vulkan-experimental\n" : L"opengl\n";
    if (!EnsureDirectoryTree(runtimeRoot + L"\\runtime") || !WriteTextFile(RendererPath(runtimeRoot), value)) {
        note = Tr(L"The renderer setting could not be saved.");
        return false;
    }
    note = TrFormat(L"Graphics renderer: {0}.", { RendererBackendName(backend) });
    WriteLogF(L"Renderer request stored: %s", RendererBackendName(backend));
    return true;
}

void LogRendererDecision(
    const std::wstring& packageDir, const std::wstring& runtimeRoot,
    const std::wstring& minecraftVersion) {
    const RendererBackend requested = RequestedRenderer(runtimeRoot);
    RendererBackend actual = RendererBackend::OpenGL;
    std::wstring fallback;
    if (requested == RendererBackend::VulkanExperimental) {
        const VulkanProbeResult probe = ProbeVulkanBridge(packageDir, runtimeRoot);
        if (probe.usable) {
            actual = RendererBackend::VulkanExperimental;
        } else {
            fallback = probe.reason;
        }
    }
    WriteLogF(L"renderer requested=%s actual=%s minecraft=%s graphics=%s",
        RendererBackendName(requested), RendererBackendName(actual),
        minecraftVersion.c_str(), DetectGraphicsRuntimeName().c_str());
    if (!fallback.empty()) {
        WriteLogF(L"renderer fallback to OpenGL: %s", fallback.c_str());
    }
    if (minecraftVersion.rfind(L"26.", 0) == 0) {
        WriteLogF(L"renderer note: 26.x may offer its own Graphics API choice; the launcher keeps its native path on OpenGL and never edits game options");
    }
}
