# vulkan_wnd_test
[Vulkan Docs issue #1039](https://github.com/KhronosGroup/Vulkan-Docs/issues/1039) repro app

### Requirements
* `Windows.h`, `vulkan.h`, c++17

This app was built with:
* Microsoft Visual Studio 2019 (v142 platform toolset, 10.0.18362.0 Windows SDK version)
* Vulkan SDK 1.1.121.2

This app is expected to be run under MSVC debugger to enable `assert()` and `DebugOutput`

This app fails on `vkCreateSwapchainKHR()` with `VK_ERROR_INITIALIZATION_FAILED` when using window with `classStyle` other than `CS_OWNDC` on Microsoft Surface Book 2 with configuration:
* Intel 620 primary gpu (with Desktop Manager)
* nVidia 1050 secondary gpu (setup as Vulkan device), various drivers versions

This app doesn't fail on other configurations tested:
* Surface Book 2 with Intel 620 as Vulkan device
* Thinkpad X1 Carbon 6th gen with secondary AMD 580 connected through Thunderbolt 3 adapter (Akitio Node) set up as Vulkan device.
* Other configurations with a single GPU.
