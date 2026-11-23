#pragma once

#include <string>

// Explicit renderer policy for the Xbox build. The default is OpenGL through
// the existing Mesa/D3D12 implementation. Vulkan is an experimental opt-in
// that stays gated behind ProbeVulkanBridge: it is never the default and is
// never used as an unverified fallback. Requesting Vulkan without a passing
// probe leaves OpenGL active and explains why.

enum class RendererBackend {
    OpenGL,
    VulkanExperimental,
};

RendererBackend RequestedRenderer(const std::wstring& runtimeRoot);
const wchar_t* RendererBackendName(RendererBackend backend);
// Persists the request. VulkanExperimental is stored only when the probe
// passes; otherwise the stored value is unchanged and note explains why.
bool SetRequestedRenderer(
    const std::wstring& runtimeRoot, const std::wstring& packageDir,
    RendererBackend backend, std::wstring& note);

struct VulkanProbeResult {
    bool usable = false;
    std::wstring reason;
    std::wstring detail;
};

VulkanProbeResult ProbeVulkanBridge(
    const std::wstring& packageDir, const std::wstring& runtimeRoot);

// Startup logging: requested backend, probe outcome, graphics runtime, and
// the backend Minecraft will actually run with. Never logs secrets.
void LogRendererDecision(
    const std::wstring& packageDir, const std::wstring& runtimeRoot,
    const std::wstring& minecraftVersion);
