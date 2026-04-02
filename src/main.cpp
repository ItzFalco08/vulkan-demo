#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <map>

constexpr int windowWidth{ 640 };
constexpr int windowHeight{ 400 };
const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class VulkanApp {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	vk::raii::Instance instance = nullptr;
	vk::raii::Context ctx; // for aceessing global vk functions
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		window = glfwCreateWindow(windowWidth, windowHeight, "vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		};
	}

	void cleanup() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	// helper functions
	void createInstance() {
		// instance layers
		std::vector<const char*> layers;
		if (enableValidationLayers) {
			layers.assign(validationLayers.begin(), validationLayers.end());
		}

		// validate instance layers
		auto validLayers = ctx.enumerateInstanceLayerProperties();
		auto invalidLayerItr = std::ranges::find_if(layers, [&validLayers](auto& layer) {
			return std::ranges::none_of(validLayers, [layer](auto& validLayer) { return strcmp(validLayer.layerName, layer) == 0; });
		});

		if (invalidLayerItr != layers.end()) {
			throw std::runtime_error("invalid layer: " + std::string(*invalidLayerItr));
		};

		// extentions
		auto extentions = getExtentions();

		// validate extentions
		auto validExtentions = ctx.enumerateInstanceExtensionProperties();
		auto invalidExtentionItr = std::ranges::find_if(extentions, [&validExtentions](auto& extention) {
			return std::ranges::none_of(validExtentions, [extention](auto& validExtention) {
				return strcmp(validExtention.extensionName, extention) == 0;
			});
		});

		if (invalidExtentionItr != extentions.end()) {
			throw std::runtime_error("Invalid extention: " + std::string(*invalidExtentionItr));
		};
		
		// vulkan instance
		constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = "Hello Triangle",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion14
		};

		vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = (uint32_t)layers.size(),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = (uint32_t)extentions.size(),
			.ppEnabledExtensionNames = extentions.data()
		};

		instance = vk::raii::Instance(ctx, createInfo);
	};
	
	void pickPhysicalDevice() {
		auto physicalDevices = instance.enumeratePhysicalDevices();

		std::multimap<int, vk::raii::PhysicalDevice> candidates;

		for (auto& physicalDevice : physicalDevices) {
			auto deviceProperties = physicalDevice.getProperties();
			auto deviceFeatures = physicalDevice.getFeatures();
			int score = 0;
			if (!deviceFeatures.geometryShader) continue;
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
			score += deviceProperties.limits.maxImageDimension2D;
			candidates.insert(std::make_pair(score, physicalDevice));
		};

		if(!candidates.empty() && candidates.rbegin()->first > 0) {
			physicalDevice = candidates.rbegin()->second;	
		}
		else {
			throw std::runtime_error("unable to find suitable gpu");
		}
	};

	// extras
	std::vector<const char*> getExtentions() {
		uint32_t extCount = 0;
		auto glfwExtentions = glfwGetRequiredInstanceExtensions(&extCount);
		std::vector<const char*> extentions(glfwExtentions, glfwExtentions + extCount);

		if (enableValidationLayers) extentions.push_back(vk::EXTDebugUtilsExtensionName);
		return extentions;
	};

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{ .messageSeverity = severityFlags,
																			  .messageType = messageTypeFlags,
																			  .pfnUserCallback = &debugCallback };
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, 
														  vk::DebugUtilsMessageTypeFlagsEXT type,
														  const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
														  void* pUserData)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

		return vk::False;
	}
};


int main() {
	std::cout << "CXX Version: " << _MSVC_LANG << std::endl;
	std::cout << "MSVC Version: " << _MSC_VER << std::endl;

	try {
		VulkanApp vkApp{};
		vkApp.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}