#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

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

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		window = glfwCreateWindow(windowWidth, windowHeight, "vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
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

	std::vector<const char*> getExtentions() {
		uint32_t extCount = 0;
		auto glfwExtentions = glfwGetRequiredInstanceExtensions(&extCount);
		std::vector<const char*> extentions(glfwExtentions, glfwExtentions + extCount);

		if (enableValidationLayers) extentions.push_back(vk::EXTDebugUtilsExtensionName);
		return extentions;
	};
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