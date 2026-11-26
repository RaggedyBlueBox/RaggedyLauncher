// Module exports for C++/WinRT
#include <winrt/base.h>

extern "C" WINRT_EXPORT HRESULT WINRT_CanUnloadNow() noexcept
{
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}

extern "C" WINRT_EXPORT HRESULT WINRT_GetActivationFactory(HSTRING activatableClassId, void** factory) noexcept
{
    return winrt::get_activation_factory(activatableClassId, factory);
}
