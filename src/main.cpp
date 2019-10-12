#include <Windows.h>
#include <exception>
#include <sstream>
#include <vector>
#include <array>
#include <cassert>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

//simple window procedure
LRESULT CALLBACK window_proc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}


//register window class helper
ATOM register_wnd_class(HINSTANCE hInstance, UINT aux_style, LPCWSTR class_name) {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW | aux_style;
	wcex.lpfnWndProc = &window_proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = class_name;
	wcex.hIconSm = NULL;
	return ::RegisterClassExW(&wcex);
}


//simple vulkan instance wrapper
//assume KHR surface extensions are present
class vulkan_instance {
public:
	vulkan_instance() {
		static constexpr std::array<const char *, 1> layers = { "VK_LAYER_LUNARG_standard_validation" };
		static constexpr std::array<const char *, 3> extensions = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult r = vkCreateInstance(&createInfo, nullptr, &inst);
		assert(r == VK_SUCCESS);
	}

	operator VkInstance() const {
		return inst;
	}

	~vulkan_instance() {
		assert(inst != VK_NULL_HANDLE);
		vkDestroyInstance(inst, nullptr);
		inst = VK_NULL_HANDLE;
	}

private:
	VkInstance inst = VK_NULL_HANDLE;
};

//simple device wrapper
//assume swapchain extension is present
class vulkan_device {
public:
	vulkan_device(VkPhysicalDevice & phy_device) {
		static constexpr std::array<const char *, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		//assume queuefamily 0 is always present
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = 0;
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.pEnabledFeatures = nullptr;

		VkResult r = vkCreateDevice(phy_device, &createInfo, nullptr, &dev);
		assert(r == VK_SUCCESS);
	}

	operator VkDevice() const {
		return dev;
	}

	~vulkan_device() {
		assert(dev != VK_NULL_HANDLE);
		vkDestroyDevice(dev, nullptr);
		dev = VK_NULL_HANDLE;
	}

private:
	VkDevice dev = VK_NULL_HANDLE;
};


//simple surface wrapper
class vulkan_surface {
public:
	vulkan_surface(HINSTANCE hInstance, HWND hWnd, VkInstance vk_instance) : vk_inst(vk_instance) {
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = hWnd;
		createInfo.hinstance = hInstance;
		VkResult r = vkCreateWin32SurfaceKHR(vk_inst, &createInfo, nullptr, &surface);
		assert(r == VK_SUCCESS);
	}

	operator VkSurfaceKHR() const {
		return surface;
	}

	~vulkan_surface() {
		assert(vk_inst != VK_NULL_HANDLE);
		vkDestroySurfaceKHR(vk_inst, surface, nullptr);
		surface = VK_NULL_HANDLE;
	}

private:
	VkInstance vk_inst;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};


//simple swapchain wrapper
class vulkan_swapchain {
public:
	vulkan_swapchain(VkDevice device, VkSurfaceKHR surface, uint32_t imageCount, VkFormat format, VkColorSpaceKHR colorSpace,
		VkExtent2D extent, VkPresentModeKHR presentMode)
		: device(device)
	{
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
		createInfo.preTransform = VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult r = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		assert(r == VK_SUCCESS); //fails here with VK_ERROR_INITIALIZATION_FAILED
	}
	~vulkan_swapchain() {
		assert(swapChain != VK_NULL_HANDLE);
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = VK_NULL_HANDLE;
	}

private:
	VkDevice device;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
};


//simple HWND wrapper
class window {
public:
	window(ATOM wnd_class, LPCWSTR title, HINSTANCE hInstance) : hWnd(::CreateWindowExW(0, (LPCWSTR) (WORD) wnd_class, title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 500, 500, NULL, NULL, hInstance, nullptr))
	{
		assert(hWnd != NULL);
	}

	operator HWND() const {
		return hWnd;
	}

	~window() {
		assert(hWnd != NULL);
		::DestroyWindow(hWnd);
		hWnd = NULL;
	}

private:
	HWND hWnd = NULL;
};

//create window, surface and swapchain
void perform_test(ATOM wnd_class, LPCWSTR title, HINSTANCE hInstance, VkInstance instance, VkPhysicalDevice phy_device, VkDevice device) {
	VkResult r = VK_SUCCESS;

	const auto wnd = window(wnd_class, title, hInstance);

	vulkan_surface surface(hInstance, wnd, instance);

	//standard capabilities check
	VkSurfaceCapabilitiesKHR capabilities;
	r = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_device, surface, &capabilities);
	assert(r == VK_SUCCESS);

	uint32_t formatCount;
	r = vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &formatCount, nullptr);
	assert(r == VK_SUCCESS);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	r = vkGetPhysicalDeviceSurfaceFormatsKHR(phy_device, surface, &formatCount, formats.data());
	assert(r == VK_SUCCESS);

	uint32_t presentModeCount;
	r = vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, surface, &presentModeCount, nullptr);
	assert(r == VK_SUCCESS);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	r = vkGetPhysicalDeviceSurfacePresentModesKHR(phy_device, surface, &presentModeCount, presentModes.data());
	assert(r == VK_SUCCESS);

	VkBool32 supported = VK_FALSE;
	r = vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, 0, surface, &supported);
	assert(r == VK_SUCCESS);
	assert(supported);

	//pick anything
	const auto imageCount = capabilities.minImageCount;
	const auto format = formats.at(0).format;
	const auto colorSpace = formats.at(0).colorSpace;
	const auto presentMode = presentModes.at(0);
	vulkan_swapchain swapchain(device, surface, imageCount, format, colorSpace, capabilities.currentExtent, presentMode);
}


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
	//number of test iterations to perform
	constexpr int n = 50;
	//index of physical device to use (as returned by vkEnumeratePhysicalDevices)
	constexpr int gpu_index = 0;
	try {
		vulkan_instance instance;
		VkResult r = VK_SUCCESS;

		uint32_t deviceCount = 0;
		r = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		assert(r == VK_SUCCESS);
		assert(deviceCount > 0);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		r = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		assert(r == VK_SUCCESS);

		VkPhysicalDevice phy_device = devices.at(gpu_index);
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(phy_device, &props);
		::OutputDebugStringA((std::ostringstream() << "Using device: " << props.deviceName << "\n").str().c_str());

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count, queue_family_properties.data());

		vulkan_device device(phy_device);

		const auto wnd1_class = register_wnd_class(hInstance, CS_OWNDC, L"classOwnDC_wnd");
		const auto wnd2_class = register_wnd_class(hInstance, 0, L"class0_wnd");
		const auto wnd3_class = register_wnd_class(hInstance, CS_PARENTDC, L"classParentDC_wnd");

		for (int i = 0; i < n; i++) {
			perform_test(wnd1_class, L"class=CS_OWNDC", hInstance, instance, phy_device, device);
		}
		::OutputDebugStringA((std::ostringstream() << "classStyle = CS_OWNDC test passed\n").str().c_str());

		for (int i = 0; i < n; i++) {
			perform_test(wnd2_class, L"class=0", hInstance, instance, phy_device, device);
		}
		::OutputDebugStringA((std::ostringstream() << "classStyle = 0 test passed\n").str().c_str());

		for (int i = 0; i < n; i++) {
			perform_test(wnd3_class, L"class=CS_PARENTDC", hInstance, instance, phy_device, device);
		}
		::OutputDebugStringA((std::ostringstream() << "classStyle = CS_PARENTDC test passed\n").str().c_str());

	} catch (std::exception & e) {
		assert(false);
		::OutputDebugStringA((std::ostringstream() << "exteption in main(): " << e.what()).str().c_str());
		return EXIT_FAILURE;
	} catch (...) {
		assert(false);
		::OutputDebugStringA("unknown exteption in main()");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}