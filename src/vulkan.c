#include "vulkan.h"

#include <libgen.h>

#include "error.h"
#include "filesystem.h"
#include "window.h"

#define vk_assert(res) if(res != VK_SUCCESS) {\
        printf("[VULKAN] %s:%d : ", __FILE__, __LINE__);\
        print_error(res); return -1;}

/* MAX is not in the POSIX standard */
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif /* MAX */

/* Don't judge me. It removes the warning. */
double log2(double __x);

struct vk_wrapper {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	uint32_t family;
	VkDevice device;
	VkQueue queue;
	VkSurfaceFormatKHR format;
	VkFormat depth_format;
	VkExtent2D win_size;
	VkSwapchainKHR swapchain;
	uint32_t image_count;
	VkImage *images;
	VkImageView *image_views;
	VkPhysicalDeviceMemoryProperties mem_props;
	VkImage depth_image;
	VkDeviceMemory depth_memory;
	VkImageView depth_view;
	VkRenderPass render_pass;
	VkFramebuffer *frame_buffers;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	VkDescriptorPool pool;
	VkSemaphore image_aquired;
	VkFence queue_submit;
	uint32_t image_index;
};

static struct vk_wrapper vk;

/*
 * Print the occured vulkan error.
 * 
 * @res: The error number
 */
static void print_error(VkResult res)
{
	char *err;

	switch (res) {
	case VK_NOT_READY:
		err = "VK_NOT_READY";
		break;
	case VK_TIMEOUT:
		err = "VK_TIMEOUT";
		break;
	case VK_EVENT_SET:
		err = "VK_EVENT_SET";
		break;
	case VK_EVENT_RESET:
		err = "VK_EVENT_RESET";
		break;
	case VK_INCOMPLETE:
		err = "VK_INCOMPLETE";
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		err = "VK_ERROR_OUT_OF_HOST_MEMORY";
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		err = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		err = "VK_ERROR_INITIALIZATION_FAILED";
		break;
	case VK_ERROR_DEVICE_LOST:
		err = "VK_ERROR_DEVICE_LOST";
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		err = "VK_ERROR_MEMORY_MAP_FAILED";
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		err = "VK_ERROR_LAYER_NOT_PRESENT";
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		err = "VK_ERROR_EXTENSION_NOT_PRESENT";
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		err = "VK_ERROR_FEATURE_NOT_PRESENT";
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		err = "VK_ERROR_INCOMPATIBLE_DRIVER";
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		err = "VK_ERROR_TOO_MANY_OBJECTS";
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		err = "VK_ERROR_FORMAT_NOT_SUPPORTED";
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		err = "VK_ERROR_FRAGMENTED_POOL";
		break;
	case VK_ERROR_UNKNOWN:
		err = "VK_ERROR_UNKNOWN";
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		err = "VK_ERROR_OUT_OF_POOL_MEMORY";
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		err = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		break;
	case VK_ERROR_FRAGMENTATION:
		err = "VK_ERROR_FRAGMENTATION";
		break;
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		err = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		err = "VK_ERROR_SURFACE_LOST_KHR";
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		err = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		break;
	case VK_SUBOPTIMAL_KHR:
		err = "VK_SUBOPTIMAL_KHR";
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		err = "VK_ERROR_OUT_OF_DATE_KHR";
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		err = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		err = "VK_ERROR_VALIDATION_FAILED_EXT";
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		err = "VK_ERROR_INVALID_SHADER_NV";
		break;
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		err = "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		break;
	case VK_ERROR_NOT_PERMITTED_EXT:
		err = "VK_ERROR_NOT_PERMITTED_EXT";
		break;
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		err = "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		break;
	default:
		err = "";
		printf("%d", res);
		break;
	}
	printf("%s\n", err);
}

/*
 * Create a vulkan instance.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_instance(void)
{
	unsigned int i;
	uint32_t layer_count;
	const char *layer;
	unsigned int ext_count;
	const char **ext;
	VkResult res;
	VkApplicationInfo app_info;
	VkLayerProperties *layers;
	VkInstanceCreateInfo create_info;

	/* Set the vulkan instance version */
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = NULL;
	app_info.applicationVersion = 0;
	app_info.pEngineName = NULL;
	app_info.engineVersion = 0;
	app_info.apiVersion = VK_API_VERSION_1_1;

	/*
	 * Check for validation layer VK_LAYER_KHRONOS_validation and use it if
	 * present.
	 */
	res = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	vk_assert(res);
	layers = malloc(sizeof(VkLayerProperties)*layer_count);
	res = vkEnumerateInstanceLayerProperties(&layer_count, layers);
	vk_assert(res);

	layer = NULL;

	for(i = 0; i < layer_count; i++) {
		if(!strcmp(layers[i].layerName, "VK_LAYER_KHRONOS_validation")) {
			layer = "VK_LAYER_KHRONOS_validation";
			break;
		}
	}
	free(layers);
#if 0
	/* Get the instance extensions */
	if(SDL_Vulkan_GetInstanceExtensions(NULL, &ext_count, NULL) < 0)
		return -1;
	
	ext = malloc(sizeof(char*) * ext_count);

	if(SDL_Vulkan_GetInstanceExtensions(NULL, &ext_count, ext) < 0)
		return -1;
#else
	ext_count = 3;
	ext = malloc(sizeof(char*) * ext_count);
	ext[0] = "VK_KHR_surface";
	ext[1] = "VK_KHR_xcb_surface";
	ext[2] = "VK_KHR_xlib_surface";
#endif

	/* Create the vulkan instance */
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledLayerCount = layer != NULL ? 1 : 0;
	create_info.ppEnabledLayerNames = layer != NULL ? &layer : NULL;
	create_info.enabledExtensionCount = ext_count;
	create_info.ppEnabledExtensionNames = ext;

	res = vkCreateInstance(&create_info, NULL, &vk.instance);
	free(ext);
	vk_assert(res);
	return 0;
}

/*
 * Create a surface to display the render on.
 * 
 * @window: A pointer to the SDL_Window the surface should be displayed on
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_surface(SDL_Window *window)
{
	return SDL_Vulkan_CreateSurface(window, vk.instance, &vk.surface);
}

/*
 * Select the best physical device (gpu) to render on.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int get_gpu(void)
{
	unsigned int i;
	uint32_t gpu_count;
	VkResult res;
	VkPhysicalDevice *gpus;
	int best_priority;

	/* Get all available gpus */
	res = vkEnumeratePhysicalDevices(vk.instance, &gpu_count, NULL);
	vk_assert(res);
	gpus = malloc(sizeof(VkPhysicalDevice) * gpu_count);
	res = vkEnumeratePhysicalDevices(vk.instance, &gpu_count, gpus);
	vk_assert(res);

	/* Prioritize a discrete gpu, more checks may follow */
	best_priority = 0;
	vk.gpu = gpus[0];
	for(i = 0; i < gpu_count; i++) {
		int priority;
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(gpus[i], &props);
		vkGetPhysicalDeviceFeatures(gpus[i], &features);
		if(props.apiVersion >= VK_API_VERSION_1_1)
			priority++;
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			priority+=2;
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			priority++;
		if(features.samplerAnisotropy)
			priority++;

		if(priority > best_priority) {
			best_priority = priority;
			vk.gpu = gpus[i];
		}	
	}

	free(gpus);

	return 0;
}

/*
 * Get the index of the best queue family, which renders.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static void get_family_index(void)
{
	unsigned int i;
	uint32_t family_count;
	VkQueueFamilyProperties* families;

	/* Get all available queue families */
	vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &family_count, NULL);
	families = malloc(sizeof(VkQueueFamilyProperties) * family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &family_count,
	                                         families);

	/* Select the queue family, which can render and display */
	vk.family = 0;
	for(i = 0; i < family_count; i++) {
		VkBool32 support;
		vkGetPhysicalDeviceSurfaceSupportKHR(vk.gpu, i, vk.surface,
		                                     &support);
		if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && support) {
			vk.family = i;
			break;
		}
	}
	free(families);
}

/*
 * Create a logical device.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_device(void)
{
	const char* extensions[1];
	float queue_prio;
	VkResult res;
	VkDeviceQueueCreateInfo queue_info;
	VkPhysicalDeviceFeatures features;
	VkDeviceCreateInfo create_info;

	queue_prio = 1.0f;

	/* Set queue info */
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.flags = 0;
	queue_info.queueFamilyIndex = vk.family;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_prio;

	/* Set the device extensions */
	extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	/* Set the optional device features */
	features.robustBufferAccess = VK_FALSE;
	features.fullDrawIndexUint32 = VK_FALSE;
	features.imageCubeArray = VK_FALSE;
	features.independentBlend = VK_FALSE;
	features.geometryShader = VK_FALSE;
	features.tessellationShader = VK_FALSE;
	features.sampleRateShading = VK_FALSE;
	features.dualSrcBlend = VK_FALSE;
	features.logicOp = VK_FALSE;
	features.multiDrawIndirect = VK_FALSE;
	features.drawIndirectFirstInstance = VK_FALSE;
	features.depthClamp = VK_FALSE;
	features.depthBiasClamp = VK_FALSE;
	features.fillModeNonSolid = VK_FALSE;
	features.depthBounds = VK_FALSE;
	features.wideLines = VK_FALSE;
	features.largePoints = VK_FALSE;
	features.alphaToOne = VK_FALSE;
	features.multiViewport = VK_FALSE;
	features.samplerAnisotropy = VK_TRUE;
	features.textureCompressionETC2 = VK_FALSE;
	features.textureCompressionASTC_LDR = VK_FALSE;
	features.textureCompressionBC = VK_FALSE;
	features.occlusionQueryPrecise = VK_FALSE;
	features.pipelineStatisticsQuery = VK_FALSE;
	features.vertexPipelineStoresAndAtomics = VK_FALSE;
	features.fragmentStoresAndAtomics = VK_FALSE;
	features.shaderTessellationAndGeometryPointSize = VK_FALSE;
	features.shaderImageGatherExtended = VK_FALSE;
	features.shaderStorageImageExtendedFormats = VK_FALSE;
	features.shaderStorageImageMultisample = VK_FALSE;
	features.shaderStorageImageReadWithoutFormat = VK_FALSE;
	features.shaderStorageImageWriteWithoutFormat = VK_FALSE;
	features.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
	features.shaderSampledImageArrayDynamicIndexing = VK_FALSE;
	features.shaderStorageBufferArrayDynamicIndexing = VK_FALSE;
	features.shaderStorageImageArrayDynamicIndexing = VK_FALSE;
	features.shaderClipDistance = VK_FALSE;
	features.shaderCullDistance = VK_FALSE;
	features.shaderFloat64 = VK_FALSE;
	features.shaderInt64 = VK_FALSE;
	features.shaderInt16 = VK_FALSE;
	features.shaderResourceResidency = VK_FALSE;
	features.shaderResourceMinLod = VK_FALSE;
	features.sparseBinding = VK_FALSE;
	features.sparseResidencyBuffer = VK_FALSE;
	features.sparseResidencyImage2D = VK_FALSE;
	features.sparseResidencyImage3D = VK_FALSE;
	features.sparseResidency2Samples = VK_FALSE;
	features.sparseResidency4Samples = VK_FALSE;
	features.sparseResidency8Samples = VK_FALSE;
	features.sparseResidency16Samples = VK_FALSE;
	features.sparseResidencyAliased = VK_FALSE;
	features.variableMultisampleRate = VK_FALSE;
	features.inheritedQueries = VK_FALSE;

	/* Create the logical device */
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.queueCreateInfoCount = 1;
	create_info.pQueueCreateInfos = &queue_info;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.enabledExtensionCount = 1;
	create_info.ppEnabledExtensionNames = extensions;
	create_info.pEnabledFeatures = &features;

	res = vkCreateDevice(vk.gpu, &create_info, NULL, &vk.device);
	vk_assert(res);
	return 0;
}

/*
 * Get the render queue.
 */
static void get_queue(void)
{
	vkGetDeviceQueue(vk.device, vk.family, 0, &vk.queue);
}

/*
 * Select the best format.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int get_format(void)
{
	uint32_t format_count;
	VkResult res;
	VkSurfaceFormatKHR* formats;

	/* Get all available formats */
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface,
	                                           &format_count, NULL);
	vk_assert(res);
	formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface,
	                                           &format_count, formats);
	
	/*
	 * If all formats are possible, use VK_FORMAT_B8G8R8A8_UNORM, else use
	 * the first format.
	 */
	if(formats[0].format == VK_FORMAT_UNDEFINED) {
		vk.format.format = VK_FORMAT_B8G8R8A8_UNORM;
		vk.format.colorSpace = formats[0].colorSpace;
	} else {
		vk.format.format = formats[0].format;
		vk.format.colorSpace = formats[0].colorSpace;
	}
	free(formats);
	return 0;
}

/*
 * Select the best format for the depth image.
 */
static void get_depth_format(void)
{
	int i;
	VkFormat formats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

	for (i = 0; i < 5; i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(vk.gpu, formats[i], &props);

		if (props.optimalTilingFeatures &
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			vk.depth_format = formats[i];
			break;
		}
	}
}

/*
 * Get the memory properties of the gpu.
 */
static void get_memory_properties(void)
{
	vkGetPhysicalDeviceMemoryProperties(vk.gpu, &vk.mem_props);
}

/*
 * Select the best type of memory for a certain use case.
 * 
 * @type_bits: The type bits from the memory requirements
 * @props: The memory properties
 * 
 * Returns: The index of the memory type
 */
static uint32_t get_memory_type(uint32_t type_bits, VkMemoryPropertyFlags props)
{
	uint32_t i;

	for(i = 0; i < vk.mem_props.memoryTypeCount; i++) {
		if((type_bits & 1) == 1) {
			if((vk.mem_props.memoryTypes[i].propertyFlags & props)
				== props) {
				return i;
			}
		}
		type_bits >>= 1;
	}
	return 0;
}

/*
 * Create an image
 *
 * @format: The format of the image
 * @width: The width of the image
 * @height: The height of the image
 * @mipLevels: The amount of mip levels
 * @arrayLayers: The amount of array layers (for cubemaps)
 * @samples: The amount of msaa
 * @usage: The usage of the image
 * @image: a pointer to the handle of the image, which the function creates
 * @memory: a pointer to the handle of the image memory, which the function
 *          creates
 *
 * Returns: 0 on success or -1 if an error occured
 */
static int create_image(VkFormat format, uint32_t width, uint32_t height,
			char changeable, uint32_t mipLevels,
			uint32_t arrayLayers, VkSampleCountFlags samples,
			VkImageUsageFlags usage, VkImage *image,
			VkDeviceMemory *memory)
{
	VkResult res;
	VkImageCreateInfo create_info;
	VkMemoryRequirements req;
	VkMemoryAllocateInfo alloc_info;

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.pNext = NULL;
	if(arrayLayers > 1) {
		create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	} else {
		create_info.flags = 0;
	}
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.format = format;
	create_info.extent.width = width;
	create_info.extent.height = height;
	create_info.extent.depth = 1;
	create_info.mipLevels = mipLevels;
	create_info.arrayLayers = arrayLayers;
	create_info.samples = samples;
	if(changeable)
		create_info.tiling = VK_IMAGE_TILING_LINEAR;
	else
		create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = NULL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	res = vkCreateImage(vk.device, &create_info, NULL, image);
	vk_assert(res);

	vkGetImageMemoryRequirements(vk.device, *image, &req);

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.allocationSize = req.size;
	if(changeable) {
		alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	} else {
		alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	res = vkAllocateMemory(vk.device, &alloc_info, NULL, memory);
	vk_assert(res);

	res = vkBindImageMemory(vk.device, *image, *memory, 0);
	vk_assert(res);

	return 0;
}

/*
 * Create an image view
 *
 * @image: The image
 * @viewType: The type of image view (2D or CUBEMAP)
 * @format: The format of the image
 * @aspect: The aspect of the image view
 * @mipLevels: The mip levels of the image
 * @arrayLayers: The amount of array layers (for cubemaps)
 * @imageView: a pointer to the handle of the image view, which the function
 *             creates
 *
 * Returns: 0 on success or -1 if an error occured
 */
static int create_image_view(VkImage image, VkFormat format,
			VkImageAspectFlags aspect, uint32_t mipLevels,
			uint32_t arrayLayers, VkImageView *imageView)
{
	VkResult res;
	VkImageViewCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.image = image;
	if(arrayLayers > 1)
		create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	create_info.subresourceRange.aspectMask = aspect;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = mipLevels;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = arrayLayers;

	res = vkCreateImageView(vk.device, &create_info, NULL, imageView);
	vk_assert(res);

	return 0;
}

static int create_sampler(char repeat, char anisotropy, float mip_levels,
			  VkSampler *sampler)
{
	VkResult res;
	VkSamplerCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.magFilter = VK_FILTER_LINEAR;
	create_info.minFilter = VK_FILTER_LINEAR;
	create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	if(repeat) {
		create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	} else {
		create_info.addressModeU =
					VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		create_info.addressModeV = 
					VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		create_info.addressModeW =
					VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}
	create_info.mipLodBias = 0.0f;
	if(anisotropy) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(vk.gpu, &props);

		create_info.anisotropyEnable = VK_TRUE;
		create_info.maxAnisotropy = props.limits.maxSamplerAnisotropy;
	} else {
		create_info.anisotropyEnable = VK_FALSE;
		create_info.maxAnisotropy = 1.0f;
	}
	create_info.compareEnable = VK_FALSE;
	create_info.compareOp = VK_COMPARE_OP_NEVER;
	create_info.minLod = 0.0f;
	create_info.maxLod = mip_levels;
	create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	create_info.unnormalizedCoordinates = VK_FALSE;

	res = vkCreateSampler(vk.device, &create_info, NULL, sampler);
	vk_assert(res);

	return 0;
}

/*
 * Create a swapchain.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_swapchain(void)
{
	uint32_t i;
	VkResult res;
	VkSurfaceCapabilitiesKHR caps;
	VkCompositeAlphaFlagBitsKHR composites[4];
	VkCompositeAlphaFlagBitsKHR composite;
	VkSwapchainCreateInfoKHR create_info;
	uint32_t present_mode_count;
	VkPresentModeKHR *present_modes;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.gpu, vk.surface,
	                                                &caps);
	vk_assert(res);

	/* Select the best composite alpha */
	composites[0] = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	composites[1] = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	composites[2] = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	composites[3] = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

	composite = composites[0];
	for(i = 0; i < 4; i++) {
		if(caps.supportedCompositeAlpha & composites[i]) {
			composite = composites[i];
			break;
		}
	}

	/* Get the current window size */
	if(caps.currentExtent.width == 0xFFFFFFFF) {
		vk.win_size.width = WIN_W;
		vk.win_size.height = WIN_H;
	} else {
		vk.win_size = caps.currentExtent;
	}

	/* Currently not relevant. Will be used to determine *no vsync*
	 * availability */
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk.gpu, vk.surface,
						&present_mode_count, NULL);
	vk_assert(res);
	present_modes = malloc(sizeof(VkPresentModeKHR)*present_mode_count);
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk.gpu, vk.surface,
					&present_mode_count, present_modes);
	free(present_modes);

	/* Create the swapchain */
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.surface = vk.surface;
	create_info.minImageCount = caps.minImageCount+1;
	create_info.imageFormat = vk.format.format;
	create_info.imageColorSpace = vk.format.colorSpace;
	create_info.imageExtent = vk.win_size;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 1;
	create_info.pQueueFamilyIndices = &vk.family;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = composite;
	create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	res = vkCreateSwapchainKHR(vk.device, &create_info, NULL,
	                           &vk.swapchain);
	vk_assert(res);
	return 0;
}

/*
 * Get the swapchain images and create the image views
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int get_swapchain_images(void)
{
	VkResult res;
	uint32_t i;

	res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.image_count,
				      NULL);
	vk_assert(res);

	vk.images = malloc(sizeof(VkImage)*vk.image_count);

	res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.image_count,
				      vk.images);
	vk_assert(res);

	vk.image_views = malloc(sizeof(VkImageView)*vk.image_count);

	for(i = 0; i < vk.image_count; i++) {
		if(create_image_view(vk.images[i], vk.format.format,
					VK_IMAGE_ASPECT_COLOR_BIT, 1, 1,
					&vk.image_views[i]) < 0) {
			return -1;
		}
	}

	return 0;
}

/*
 * Create the depth buffer.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_depth_buffer(void)
{

	if(create_image(vk.depth_format, vk.win_size.width, vk.win_size.height,
			0, 1, 1, VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			&vk.depth_image, &vk.depth_memory) < 0) {
		return -1;
	}

	if(create_image_view(vk.depth_image, vk.depth_format,
				VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1,
				&vk.depth_view) < 0) {
		return -1;
	}

	return 0;
}

/*
 * Create the render pass.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_render_pass(void)
{
	VkResult res;
	VkAttachmentDescription attachments[2];
	VkAttachmentReference color_reference;
	VkAttachmentReference depth_reference;
	VkSubpassDescription subpass;
	VkSubpassDependency dependency;
	VkRenderPassCreateInfo create_info;

	/* Color output */
	attachments[0].flags = 0;
	attachments[0].format = vk.format.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	/* Depth output */
	attachments[1].flags = 0;
	attachments[1].format = vk.depth_format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	depth_reference.attachment = 1;
	depth_reference.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	/* Create the subpass */
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depth_reference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	/* Create the render pass */
	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.attachmentCount = 2;
	create_info.pAttachments = attachments;
	create_info.subpassCount = 1;
	create_info.pSubpasses = &subpass;
	create_info.dependencyCount = 1;
	create_info.pDependencies = &dependency;

	res = vkCreateRenderPass(vk.device, &create_info, NULL,
	                         &vk.render_pass);
	vk_assert(res);
	return 0;
}

/*
 * Create framebuffers from the image views of the swapchain images.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_framebuffers(void)
{
	unsigned int i;
	VkResult res;
	VkImageView attachments[2];
	VkFramebufferCreateInfo create_info;

	vk.frame_buffers = malloc(sizeof(VkFramebuffer)*vk.image_count);

	attachments[1] = vk.depth_view;

	create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.renderPass = vk.render_pass;
	create_info.attachmentCount = 2;
	create_info.pAttachments = attachments;
	create_info.width = vk.win_size.width;
	create_info.height = vk.win_size.height;
	create_info.layers = 1;

	for(i = 0; i < vk.image_count; i++) {
		attachments[0] = vk.image_views[i];
		res = vkCreateFramebuffer(vk.device, &create_info, NULL,
		                          &vk.frame_buffers[i]);
		vk_assert(res);
	}
	return 0;
}

/*
 * Create a command pool, which contains the command buffer.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_command_pool(void)
{
	VkResult res;
	VkCommandPoolCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = vk.family;

	res = vkCreateCommandPool(vk.device, &create_info, NULL,
	                          &vk.command_pool);
	vk_assert(res);
	return 0;
}

/*
 * Allocate a command buffer, which records the render commands.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int allocate_command_buffer(void)
{
	VkResult res;
	VkCommandBufferAllocateInfo alloc_info;

	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.commandPool = vk.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	res = vkAllocateCommandBuffers(vk.device, &alloc_info,
	                               &vk.command_buffer);
	vk_assert(res);
	return 0;
}

/*
 * Create a descriptor pool, which contains the descriptor sets.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_descriptor_pool(void)
{
	VkResult res;
	VkDescriptorPoolSize sizes[2];
	VkDescriptorPoolCreateInfo create_info;

	/* Set the types and amounts of the descriptors */
	sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sizes[0].descriptorCount = 10;
	sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[1].descriptorCount = 10;

	/* Create the descriptor pool */
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	create_info.maxSets = 128;
	create_info.poolSizeCount = 2;
	create_info.pPoolSizes = sizes;

	res = vkCreateDescriptorPool(vk.device, &create_info, NULL, &vk.pool);
	vk_assert(res);
	return 0;
}

/*
 * Create a semaphore.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_semaphore(void)
{
	VkResult res;
	VkSemaphoreCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;

	res = vkCreateSemaphore(vk.device, &create_info, NULL,
	                        &vk.image_aquired);
	vk_assert(res);

	return 0;
}

/*
 * Create a fence.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_fence(void)
{
	VkResult res;
	VkFenceCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;

	res = vkCreateFence(vk.device, &create_info, NULL, &vk.queue_submit);
	vk_assert(res);
	return 0;
}

/*
 * Create a shader.
 * 
 * @path: the path to the SPIR-V shader
 * @shd: a pointer to the handle of the shader module, which the function
 *       creates
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_shader(char *path, VkShaderModule *shd)
{
	uint32_t* code;
	long size;
	VkResult res;
	VkShaderModuleCreateInfo create_info;

	if(fs_load_file(path, (uint8_t**)&code, &size) < 0) {
		ERR_LOG(("Failed to load shader: %s", path));
		return -1;
	}

	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.codeSize = (size_t)size;
	create_info.pCode = code;

	res = vkCreateShaderModule(vk.device, &create_info, NULL, shd);
	free(code);
	vk_assert(res);
	return 0;
}

/*
 * Create a descriptor set layout for the pipeline layout.
 * Currently the set layout is the same for all pipelines, will be configurable
 * in the future.
 * 
 * @set_layout: a pointer to the handle of the set layout, which the function
 *              creates
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_set_layout(VkDescriptorSetLayout *set_layout)
{
	VkResult res;
	VkDescriptorSetLayoutBinding bindings[2];
	VkDescriptorSetLayoutCreateInfo create_info;

	/* Determine, where each descriptor should be in the shaders */
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = NULL;
	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = NULL;

	/* Create the descriptor set layout */
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.bindingCount = 2;
	create_info.pBindings = bindings;

	res = vkCreateDescriptorSetLayout(vk.device, &create_info, NULL,
	                                  set_layout);
	vk_assert(res);
	return 0;
}

/*
 * Create a pipeline layout based on the descriptor set layout.
 * 
 * @set_layout: a pointer to the descriptor set layout
 * @layout: a pointer to the handle of the pipeline layout, which the function
 *          creates
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_pipeline_layout(VkDescriptorSetLayout *set_layout,
                                  VkPipelineLayout *layout)
{
	VkResult res;
	VkPipelineLayoutCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.setLayoutCount = 1;
	create_info.pSetLayouts = set_layout;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;

	res = vkCreatePipelineLayout(vk.device, &create_info, NULL, layout);
	vk_assert(res);
	return 0;
}

/*
 * Get the name of the pipeline cache file generated from the file name of the
 * vertex shader.
 * 
 * @vs: The path to the vertex shader file
 * 
 * Returns: The pipeline cache file name
 */
static char *get_cache_file_name(char *vs)
{
	char *vs_copy;
	char *pipeline_name;
	char *dot;
	char *res;

	vs_copy = malloc(strlen(vs)+1);
	strcpy(vs_copy, vs);

	pipeline_name = basename(vs_copy);
	dot = strchr(pipeline_name, '.');
	pipeline_name[dot-pipeline_name] = '\0';

	res = malloc(strlen(pipeline_name)+12);
	memcpy(res, ".cache/", 8);
	strcpy(res+7, pipeline_name);
	memcpy(res+strlen(pipeline_name)+7, ".bin", 5);
	return res;
}

/*
 * Create a pipeline cache by trying to load the cache file or create a new one
 * 
 * @name: The path to the pipeline cache file
 * @cache: a pointer to the handle of the pipeline cache, which the function
 *         creates
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int create_pipeline_cache(char *name, VkPipelineCache *cache)
{
	uint8_t *buf;
	long len;
	VkResult res;
	VkPipelineCacheCreateInfo create_info;

	buf = NULL;
	len = 0;

	fs_load_file(name, &buf, &len);

	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.initialDataSize = len;
	create_info.pInitialData = buf;

	res = vkCreatePipelineCache(vk.device, &create_info, NULL, cache);
	vk_assert(res);
	free(buf);
	return 0;
}

/*
 * Save the pipeline chache data into a file
 * 
 * @name: The path to the pipeline cache file
 * @cache: The pipeline cache
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int save_cache(char *name, VkPipelineCache cache)
{
	VkResult res;
	size_t len;
	uint8_t *data;

	res = vkGetPipelineCacheData(vk.device, cache, &len, NULL);
	vk_assert(res);
	data = malloc(len);
	res = vkGetPipelineCacheData(vk.device, cache, &len, data);
	vk_assert(res);

	fs_create_dir(".cache");
	fs_write_file(name, data, len);

	free(data);
	return 0;
}

extern int vk_create_pipeline(char *vtx, char *frg, enum vk_in_attr attr,
                              enum mdl_type type, struct vk_pipeline *pipeline)
{
	int counter;
	uint32_t bin_size;
	VkResult res;
	VkShaderModule modules[2];
	VkPipelineShaderStageCreateInfo shaders[2];
	VkVertexInputBindingDescription in_bin;
	VkVertexInputAttributeDescription in_attr[5];
	VkPipelineVertexInputStateCreateInfo vtx_input;
	VkPipelineInputAssemblyStateCreateInfo in_as;
	VkPipelineTessellationStateCreateInfo tes;
	VkPipelineViewportStateCreateInfo view;
	VkPipelineRasterizationStateCreateInfo ras;
	VkPipelineMultisampleStateCreateInfo multi;
	VkPipelineDepthStencilStateCreateInfo depth;
	VkPipelineColorBlendAttachmentState color_state;
	VkPipelineColorBlendStateCreateInfo color;
	VkDynamicState dynamic[2];
	VkPipelineDynamicStateCreateInfo dyn;
	char *cache_name;
	VkPipelineCache cache;
	VkGraphicsPipelineCreateInfo create_info;

	/* Create vertex and fragment shader modules */
	if(create_shader(vtx, &modules[0]) < 0)
		return -1;

	if(create_shader(frg, &modules[1]) < 0) {
		vkDestroyShaderModule(vk.device, modules[0], NULL);
		return -1;
	}

	shaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaders[0].pNext = NULL;
	shaders[0].flags = 0;
	shaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaders[0].module = modules[0];
	shaders[0].pName = "main";
	shaders[0].pSpecializationInfo = NULL;

	shaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaders[1].pNext = NULL;
	shaders[1].flags = 0;
	shaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaders[1].module = modules[1];
	shaders[1].pName = "main";
	shaders[1].pSpecializationInfo = NULL;

	/* Dynamically set the vertex input attributes */
	counter = 0;
	bin_size = 0;

	if((attr >> 0) & 1) {

		in_attr[counter].location = counter;
		in_attr[counter].binding = 0;
		in_attr[counter].format = VK_FORMAT_R32G32B32_SFLOAT;
		in_attr[counter].offset = bin_size;

		counter++;
		bin_size += 12;
	}

	if((attr >> 1) & 1) {
	
		in_attr[counter].location = counter;
		in_attr[counter].binding = 0;
		in_attr[counter].format = VK_FORMAT_R32G32_SFLOAT;
		in_attr[counter].offset = bin_size;

		counter++;
		bin_size += 8;
	}

	if((attr >> 2) & 1) {

		in_attr[counter].location = counter;
		in_attr[counter].binding = 0;
		in_attr[counter].format = VK_FORMAT_R32G32B32_SFLOAT;
		in_attr[counter].offset = bin_size;

		counter++;
		bin_size += 12;
	}

	if((attr >> 3) & 1) {

		in_attr[counter].location = counter;
		in_attr[counter].binding = 0;
		in_attr[counter].format = VK_FORMAT_R32G32B32A32_SINT;
		in_attr[counter].offset = bin_size;

		counter++;
		bin_size += 16;
	}

	if((attr >> 4) & 1) {

		in_attr[counter].location = counter;
		in_attr[counter].binding = 0;
		in_attr[counter].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		in_attr[counter].offset = bin_size;

		counter++;
		bin_size += 16;
	}

	in_bin.binding = 0;
	in_bin.stride = bin_size;
	in_bin.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	vtx_input.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vtx_input.pNext = NULL;
	vtx_input.flags = 0;
	vtx_input.vertexBindingDescriptionCount = 1;
	vtx_input.pVertexBindingDescriptions = &in_bin;
	vtx_input.vertexAttributeDescriptionCount = counter;
	vtx_input.pVertexAttributeDescriptions = in_attr;

	/* Set the input assembly properties */
	in_as.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	in_as.pNext = NULL;
	in_as.flags = 0;
	in_as.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	in_as.primitiveRestartEnable = VK_FALSE;

	/* Set the tesselation properties */
	tes.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tes.pNext = NULL;
	tes.flags = 0;
	tes.patchControlPoints = 0;

	/* Set the viewport, (is dynamic and will be set during recording) */
	view.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	view.pNext = NULL;
	view.flags = 0;
	view.viewportCount = 1;
	view.pViewports = NULL;
	view.scissorCount = 1;
	view.pScissors = NULL;

	/* Set rasterization properties */
	ras.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ras.pNext = NULL;
	ras.flags = 0;
	ras.depthClampEnable = VK_FALSE;
	ras.rasterizerDiscardEnable = VK_FALSE;
	ras.polygonMode = VK_POLYGON_MODE_FILL;
	if(type == MDL_TYPE_SKYBOX)
		ras.cullMode = VK_CULL_MODE_FRONT_BIT;
	else
		ras.cullMode = VK_CULL_MODE_BACK_BIT;
	ras.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	ras.depthBiasEnable = VK_FALSE;
	ras.depthBiasConstantFactor = 0.0f;
	ras.depthBiasClamp = 0.0f;
	ras.depthBiasSlopeFactor = 0.0f;
	ras.lineWidth = 1.0f;

	/* Set multisampling properties */
	multi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi.pNext = NULL;
	multi.flags = 0;
	multi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multi.sampleShadingEnable = VK_FALSE;
	multi.minSampleShading = 0;
	multi.pSampleMask = NULL;
	multi.alphaToCoverageEnable = VK_FALSE;
	multi.alphaToOneEnable = VK_FALSE;

	/* Set depth stencil properties */
	depth.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth.pNext = NULL;
	depth.flags = 0;
	if(type == MDL_TYPE_SKYBOX) {
		depth.depthTestEnable = VK_FALSE;
		depth.depthWriteEnable = VK_FALSE;
	} else {
		depth.depthTestEnable = VK_TRUE;
		depth.depthWriteEnable = VK_TRUE;
	}
	depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth.depthBoundsTestEnable = VK_FALSE;
	depth.stencilTestEnable = VK_FALSE;
	depth.back.failOp = VK_STENCIL_OP_KEEP;
	depth.back.passOp = VK_STENCIL_OP_KEEP;
	depth.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depth.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depth.back.compareMask = 0;
	depth.back.writeMask = 0;
	depth.back.reference = 0;
	depth.front = depth.back;
	depth.minDepthBounds = 0;
	depth.maxDepthBounds = 0;

	/* Set color blend properties */
	color_state.blendEnable = VK_FALSE;
	color_state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_state.alphaBlendOp = VK_BLEND_OP_ADD;
	color_state.colorWriteMask = 0xf;

	color.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color.pNext = NULL;
	color.flags = 0;
	color.logicOpEnable = VK_FALSE;
	color.logicOp = VK_LOGIC_OP_NO_OP;
	color.attachmentCount = 1;
	color.pAttachments = &color_state;
	color.blendConstants[0] = 1.0f;
	color.blendConstants[1] = 1.0f;
	color.blendConstants[2] = 1.0f;
	color.blendConstants[3] = 1.0f;

	/* Set dynamic properties */
	dynamic[0] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamic[1] = VK_DYNAMIC_STATE_SCISSOR;

	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.pNext = NULL;
	dyn.flags = 0;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynamic;

	cache_name = get_cache_file_name(vtx);

	/* Create layouts */
	if(create_set_layout(&pipeline->set_layout) < 0) {
		res = VK_ERROR_UNKNOWN;
		goto err;
	}

	if(create_pipeline_layout(&pipeline->set_layout, &pipeline->layout)
	   < 0) {
		res = VK_ERROR_UNKNOWN;
		goto err_destroy_set_layout;
	}

	if(create_pipeline_cache(cache_name, &cache) < 0) {
		res = VK_ERROR_UNKNOWN;
		goto err_destroy_layout;
	}

	/* Create the graphics pipeline */
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.stageCount = 2;
	create_info.pStages = shaders;
	create_info.pVertexInputState = &vtx_input;
	create_info.pInputAssemblyState = &in_as;
	create_info.pTessellationState = &tes;
	create_info.pViewportState = &view;
	create_info.pRasterizationState = &ras;
	create_info.pMultisampleState = &multi;
	create_info.pDepthStencilState = &depth;
	create_info.pColorBlendState = &color;
	create_info.pDynamicState = &dyn;
	create_info.layout = pipeline->layout;
	create_info.renderPass = vk.render_pass;
	create_info.subpass = 0;
	create_info.basePipelineHandle = VK_NULL_HANDLE;
	create_info.basePipelineIndex = 0;

	res = vkCreateGraphicsPipelines(vk.device, cache, 1,
	                                &create_info, NULL,
	                                &pipeline->pipeline);

	save_cache(cache_name, cache);
	vkDestroyPipelineCache(vk.device, cache, NULL);
	goto err;

err_destroy_layout:
	vkDestroyPipelineLayout(vk.device, pipeline->layout, NULL);
err_destroy_set_layout:
	vkDestroyDescriptorSetLayout(vk.device, pipeline->set_layout, NULL);
err:
	free(cache_name);
	vkDestroyShaderModule(vk.device, modules[0], NULL);
	vkDestroyShaderModule(vk.device, modules[1], NULL);
	vk_assert(res);
	return 0;
}


extern void vk_destroy_pipeline(struct vk_pipeline pipeline)
{
	vkDestroyPipeline(vk.device, pipeline.pipeline, NULL);
	vkDestroyPipelineLayout(vk.device, pipeline.layout, NULL);
	vkDestroyDescriptorSetLayout(vk.device, pipeline.set_layout, NULL);
}

/*
 * Allocate a descriptor set based on the descriptor set layout.
 * 
 * @set_layout: a pointer to the descriptor set layout
 * @set: a pointer to the handle of the descriptor set, which the function
 *       creates
 * 
 * Returns: 0 on success or -1 if an error occured
 */
static int allocate_descriptor_set(VkDescriptorSetLayout *set_layout,
                                   VkDescriptorSet *set)
{
	VkResult res;
	VkDescriptorSetAllocateInfo alloc_info;

	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.descriptorPool = vk.pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = set_layout;

	res = vkAllocateDescriptorSets(vk.device, &alloc_info, set);
	vk_assert(res);
	return 0;
}

extern int vk_create_constant_data(struct vk_pipeline pipeline,
                                   VkDescriptorSet *set)
{
	return allocate_descriptor_set(&pipeline.set_layout, set);
}


extern int vk_destroy_constant_data(VkDescriptorSet set)
{
	VkResult res;
	res = vkFreeDescriptorSets(vk.device, vk.pool, 1, &set);
	vk_assert(res);
	return 0;
}


extern int vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            char staging, struct vk_buffer *buffer)
{
	VkResult res;
	VkBufferCreateInfo create_info;
	VkMemoryRequirements req;
	VkMemoryAllocateInfo alloc_info;

	buffer->size = size;

	/* Create the buffer */
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.size = size;
	create_info.usage = usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = NULL;

	res = vkCreateBuffer(vk.device, &create_info, NULL, &buffer->buffer);
	vk_assert(res);

	/* Allocate the buffer memory */
	vkGetBufferMemoryRequirements(vk.device, buffer->buffer, &req);

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.allocationSize = req.size;
	if(staging)
		alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
		                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	else
		alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
		                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	res = vkAllocateMemory(vk.device, &alloc_info, NULL, &buffer->memory);
	vk_assert(res);

	res = vkBindBufferMemory(vk.device, buffer->buffer, buffer->memory, 0);
	vk_assert(res);

	if(staging) {
		res = vkMapMemory(vk.device, buffer->memory, 0, size, 0,
		                  (void **)&buffer->data);
		vk_assert(res);
	}

	return 0;
}


extern void vk_copy_data_to_buffer(void* data, struct vk_buffer buffer)
{
	if(buffer.buffer != VK_NULL_HANDLE)
		memcpy(buffer.data, data, buffer.size);
}


extern void vk_destroy_buffer(struct vk_buffer buffer)
{
	vkDestroyBuffer(vk.device, buffer.buffer, NULL);
	vkFreeMemory(vk.device, buffer.memory, NULL);
}


extern int vk_create_texture(char *pth, struct vk_texture *texture)
{
	int w, h;
	uint32_t i;
	uint8_t *buf;
	VkResult res;
	struct vk_buffer staging;
	VkCommandBufferBeginInfo begin_info;
	VkImageMemoryBarrier barrier;
	VkBufferImageCopy copy;
	VkSubmitInfo submit;

	/* Load the png and load the data into a staging buffer */
	if(fs_load_png(pth, &buf, &w, &h) < 0) {
		ERR_LOG(("Failed to load texture: %s", pth));
		return -1;
	}

	texture->mip_levels = floor(log2(MAX(w, h)))+1;

	if(vk_create_buffer(w*h*4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1,
	                    &staging) < 0) {
		ERR_LOG(("Failed to create staging buffer"));
		return -1;
	}
	memcpy(staging.data, buf, w*h*4);
	vkUnmapMemory(vk.device, staging.memory);
	free(buf);

	if(create_image(VK_FORMAT_R8G8B8A8_SRGB, w, h, 0,
			texture->mip_levels, 1, VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT, &texture->image,
			&texture->memory) < 0) {
		return -1;
	}

	/* Copy the data from the buffer to the image */
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = texture->image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
	                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
	                     NULL, 1, &barrier);

	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageOffset.x = 0;
	copy.imageOffset.y = 0;
	copy.imageOffset.z = 0;
	copy.imageExtent.width = w;
	copy.imageExtent.height = h;
	copy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(vk.command_buffer, staging.buffer,
	                       texture->image,
	                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
	                     0, NULL, 1, &barrier);

	/* Generate Mip Maps */
	for(i = 1; i < texture->mip_levels; i++) {
		VkImageBlit image_blit;

		image_blit.srcSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit.srcSubresource.mipLevel = i - 1;
		image_blit.srcSubresource.baseArrayLayer = 0;
		image_blit.srcSubresource.layerCount = 1;
		image_blit.srcOffsets[0].x = 0;
		image_blit.srcOffsets[0].y = 0;
		image_blit.srcOffsets[0].z = 0;
		image_blit.srcOffsets[1].x = w >> (i - 1);
		image_blit.srcOffsets[1].y = h >> (i - 1);
		image_blit.srcOffsets[1].z = 1;
		image_blit.dstSubresource.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit.dstSubresource.mipLevel = i;
		image_blit.dstSubresource.baseArrayLayer = 0;
		image_blit.dstSubresource.layerCount = 1;
		image_blit.dstOffsets[0].x = 0;
		image_blit.dstOffsets[0].y = 0;
		image_blit.dstOffsets[0].z = 0;
		image_blit.dstOffsets[1].x = w >> i;
		image_blit.dstOffsets[1].y = h >> i;
		image_blit.dstOffsets[1].z = 1;

		barrier.subresourceRange.baseMipLevel = i;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier(vk.command_buffer,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
		                     0, NULL, 1, &barrier);

		vkCmdBlitImage(vk.command_buffer, texture->image,
		               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		               texture->image,
		               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
		               &image_blit, VK_FILTER_LINEAR);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		vkCmdPipelineBarrier(vk.command_buffer,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL,
		                     0, NULL, 1, &barrier);
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = texture->mip_levels;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL,
	                     0, NULL, 1, &barrier);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = NULL;
	submit.waitSemaphoreCount = 0;
	submit.pWaitSemaphores = NULL;
	submit.pWaitDstStageMask = NULL;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &vk.command_buffer;
	submit.signalSemaphoreCount = 0;
	submit.pSignalSemaphores = NULL;;

	res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
	vk_assert(res);

	res = vkQueueWaitIdle(vk.queue);
	vk_assert(res);

	res = vkResetCommandPool(vk.device, vk.command_pool, 0);
	vk_assert(res);

	vk_destroy_buffer(staging);

	if(create_image_view(texture->image, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT, texture->mip_levels, 1,
			&texture->image_view) < 0) {
		return -1;
	}

	if(create_sampler(1, 1, texture->mip_levels, &texture->sampler)
			< 0) {
		return -1;
	}

	return 0;
}


extern void vk_destroy_texture(struct vk_texture texture)
{
	vkDestroySampler(vk.device, texture.sampler, NULL);
	vkDestroyImageView(vk.device, texture.image_view, NULL);
	vkDestroyImage(vk.device, texture.image, NULL);
	vkFreeMemory(vk.device, texture.memory, NULL);
}


extern int vk_create_skybox(char *pths[6], struct vk_texture *skybox)
{
	uint32_t i;
	uint8_t *buf[6];
	int w;
	int h;
	VkResult res;
	struct vk_buffer staging;
	VkCommandBufferBeginInfo begin_info;
	VkImageMemoryBarrier barrier;
	VkBufferImageCopy copies[6];
	VkSubmitInfo submit;

	for(i = 0; i < 6; i++) {
		if(fs_load_png(pths[i], &buf[i], &w, &h) < 0) {
			ERR_LOG(("Failed to load texture: %s", pths[i]));
			return -1;
		}
	}

	if(vk_create_buffer(w*h*4*6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1,
				&staging) < 0) {
		ERR_LOG(("Failed to create staging buffer"));
		return -1;
	}

	for(i = 0; i < 6; i++) {
		memcpy(staging.data+(w*h*4*i), buf[i], w*h*4);
		free(buf[i]);
	}
	vkUnmapMemory(vk.device, staging.memory);
	
	if(create_image(VK_FORMAT_R8G8B8A8_SRGB, w, h, 0, 1, 6,
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT, &skybox->image,
			&skybox->memory) < 0) {
		return -1;
	}

	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = skybox->image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 6;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
				NULL, 1, &barrier);

	for(i = 0; i < 6; i++) {
		copies[i].bufferOffset = w*h*4*i;
		copies[i].bufferRowLength = 0;
		copies[i].bufferImageHeight = 0;
		copies[i].imageSubresource.aspectMask = 
						VK_IMAGE_ASPECT_COLOR_BIT;
		copies[i].imageSubresource.mipLevel = 0;
		copies[i].imageSubresource.baseArrayLayer = i;
		copies[i].imageSubresource.layerCount = 1;
		copies[i].imageOffset.x = 0;
		copies[i].imageOffset.y = 0;
		copies[i].imageOffset.z = 0;
		copies[i].imageExtent.width = w;
		copies[i].imageExtent.height = h;
		copies[i].imageExtent.depth = 1;
	}

	vkCmdCopyBufferToImage(vk.command_buffer, staging.buffer, skybox->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6,
				copies);

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
				NULL, 0, NULL, 1, &barrier);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = NULL;
	submit.waitSemaphoreCount = 0;
	submit.pWaitSemaphores = NULL;
	submit.pWaitDstStageMask = NULL;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &vk.command_buffer;
	submit.signalSemaphoreCount = 0;
	submit.pSignalSemaphores = NULL;

	res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
	vk_assert(res);

	res = vkQueueWaitIdle(vk.queue);
	vk_assert(res);

	res = vkResetCommandPool(vk.device, vk.command_pool, 0);
	vk_assert(res);

	vk_destroy_buffer(staging);

	if(create_image_view(skybox->image, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT, 1, 6, &skybox->image_view)
			 < 0) {
		return -1;
	}

	if(create_sampler(0, 0, 1.0f, &skybox->sampler) < 0) {
		return -1;
	}

	return 0;
}


extern int vk_create_ui(int width, int height, void *pixels, void **data,
			struct vk_texture *texture)
{
	VkResult res;
	VkCommandBufferBeginInfo begin_info;
	VkImageMemoryBarrier barrier;
	VkSubmitInfo submit;

	texture->mip_levels = 1;

	if(create_image(VK_FORMAT_R8G8B8A8_SRGB, width, height, 1, 1, 1,
			VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT,
			&texture->image, &texture->memory) < 0) {
		return -1;
	}

	res = vkMapMemory(vk.device, texture->memory, 0, 4 * width * height, 0,
				data);
	vk_assert(res);

	if(pixels) {
		memcpy(*data, pixels, 4 * width * height);

		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.pNext = NULL;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = NULL;

		res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
		vk_assert(res);

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = NULL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = 0;
		barrier.dstQueueFamilyIndex = 0;
		barrier.image = texture->image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
				NULL, 1, &barrier);

		res = vkEndCommandBuffer(vk.command_buffer);
		vk_assert(res);

		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = NULL;
		submit.waitSemaphoreCount = 0;
		submit.pWaitSemaphores = NULL;
		submit.pWaitDstStageMask = NULL;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &vk.command_buffer;
		submit.signalSemaphoreCount = 0;
		submit.pSignalSemaphores = NULL;

		res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
		vk_assert(res);

		res = vkQueueWaitIdle(vk.queue);
		vk_assert(res);

		res = vkResetCommandBuffer(vk.command_buffer, 0);
		vk_assert(res);
	}

	if(create_image_view(texture->image, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT, 1, 1,
			&texture->image_view) < 0) {
		return -1;
	}

	if(create_sampler(1, 0, 1, &texture->sampler) < 0) {
		return -1;
	}

	return 0;
}


extern int vk_update_texture(int width, int height, void *pixels, void *data,
			     struct vk_texture texture)
{
	VkResult res;
	VkCommandBufferBeginInfo begin_info;
	VkImageMemoryBarrier barrier;
	VkSubmitInfo submit;

	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = 0;
	barrier.dstQueueFamilyIndex = 0;
	barrier.image = texture.image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
			NULL, 1, &barrier);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = NULL;
	submit.waitSemaphoreCount = 0;
	submit.pWaitSemaphores = NULL;
	submit.pWaitDstStageMask = NULL;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &vk.command_buffer;
	submit.signalSemaphoreCount = 0;
	submit.pSignalSemaphores = NULL;

	res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
	vk_assert(res);

	res = vkQueueWaitIdle(vk.queue);
	vk_assert(res);

	res = vkResetCommandBuffer(vk.command_buffer, 0);
	vk_assert(res);

	memcpy(data, pixels, width * height * 4);

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
			NULL, 1, &barrier);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

	res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
	vk_assert(res);

	res = vkQueueWaitIdle(vk.queue);
	vk_assert(res);

	res = vkResetCommandBuffer(vk.command_buffer, 0);
	vk_assert(res);

	return 0;
}


extern int vk_init(SDL_Window* window)
{
	uint32_t i;

	if(create_instance() < 0)
		goto err;
	
	if(create_surface(window) < 0)
		goto err_instance;

	if(get_gpu() < 0)
		goto err_surface;

	get_family_index();

	if(create_device() < 0)
		goto err_surface;

	get_queue();

	if(get_format() < 0)
		goto err_device;

	get_depth_format();

	if(create_swapchain() < 0)
		goto err_device;

	get_memory_properties();

	if(get_swapchain_images() < 0)
		goto err_swapchain;

	if(create_depth_buffer() < 0)
		goto err_swapchain_images;

	if(create_render_pass() < 0)
		goto err_depth_buffer;

	if(create_framebuffers() < 0)
		goto err_render_pass;

	if(create_command_pool() < 0)
		goto err_framebuffers;

	if(allocate_command_buffer() < 0)
		goto err_command_pool;

	if(create_descriptor_pool() < 0)
		goto err_command_pool;

	if(create_semaphore() < 0)
		goto err_descriptor_pool;

	if(create_fence() < 0)
		goto err_semaphore;

	return 0;

err_semaphore:
	vkDestroySemaphore(vk.device, vk.image_aquired, NULL);
err_descriptor_pool:
	vkDestroyDescriptorPool(vk.device, vk.pool, NULL);
err_command_pool:
	vkDestroyCommandPool(vk.device, vk.command_pool, NULL);
err_framebuffers:
	for(i = 0; i < vk.image_count; i++)
		vkDestroyFramebuffer(vk.device, vk.frame_buffers[i], NULL);
	free(vk.frame_buffers);
err_render_pass:
	vkDestroyRenderPass(vk.device, vk.render_pass, NULL);
err_depth_buffer:
	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
err_swapchain_images:
	for(i = 0; i < vk.image_count; i++)
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	free(vk.image_views);
	free(vk.images);
err_swapchain:
	vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
err_device:
	vkDestroyDevice(vk.device, NULL);
err_surface:
	vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);
err_instance:
	vkDestroyInstance(vk.instance, NULL);
err:
	return -1;
}


extern void vk_destroy(void)
{
	uint32_t i;

	vkDestroyFence(vk.device, vk.queue_submit, NULL);
	vkDestroySemaphore(vk.device, vk.image_aquired, NULL);
	vkDestroyDescriptorPool(vk.device, vk.pool, NULL);
	vkDestroyCommandPool(vk.device, vk.command_pool, NULL);
	vkDestroyRenderPass(vk.device, vk.render_pass, NULL);
	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	for(i = 0; i < vk.image_count; i++) {
		vkDestroyFramebuffer(vk.device, vk.frame_buffers[i], NULL);
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	}
	free(vk.frame_buffers);
	free(vk.image_views);
	free(vk.images);
	vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
	vkDestroyDevice(vk.device, NULL);
	vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);
	vkDestroyInstance(vk.instance, NULL);
}


extern int vk_resize(void)
{
	uint32_t i;

	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	for(i = 0; i < vk.image_count; i++) {
		vkDestroyFramebuffer(vk.device, vk.frame_buffers[i], NULL);
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	}
	free(vk.frame_buffers);
	free(vk.image_views);
	free(vk.images);
	vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);

	if(create_swapchain() < 0)
		goto err;

	if(get_swapchain_images() < 0)
		goto err_swapchain;

	if(create_depth_buffer() < 0)
		goto err_swapchain_images;

	if(create_framebuffers() < 0)
		goto err_depth_buffer;

	return 0;

err_depth_buffer:
	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
err_swapchain_images:
	for(i = 0; i < vk.image_count; i++)
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	free(vk.image_views);
	free(vk.images);
err_swapchain:
	vkDestroySwapchainKHR(vk.device, vk.swapchain, NULL);
err:
	return -1;
}


extern int vk_set_uniform_buffer(struct vk_buffer buffer, VkDescriptorSet set)
{
	VkDescriptorBufferInfo buffer_info;
	VkWriteDescriptorSet write;

	if(!buffer.buffer)
		return 0;

	buffer_info.buffer = buffer.buffer;
	buffer_info.offset = 0;
	buffer_info.range = buffer.size;

	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstSet = set;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.pImageInfo = NULL;
	write.pBufferInfo = &buffer_info;
	write.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(vk.device, 1, &write, 0, NULL);
	return 0;
}


extern int vk_set_texture(struct vk_texture texture, VkDescriptorSet set)
{
	VkDescriptorImageInfo image_info;
	VkWriteDescriptorSet write;

	image_info.sampler = texture.sampler;
	image_info.imageView = texture.image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = NULL;
	write.dstSet = set;
	write.dstBinding = 1;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &image_info;
	write.pBufferInfo = NULL;
	write.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(vk.device, 1, &write, 0, NULL);
	return 0;
}


extern int vk_render_start(void)
{
	int height;
	VkResult res;
	VkCommandBufferBeginInfo begin_info;
	VkClearValue clear_values[2];
	VkRenderPassBeginInfo pass_begin;
	VkViewport viewport;
	VkRect2D scissor;

	/* Get the swapchain image, it should render to */
	res = vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX,
	                            vk.image_aquired, VK_NULL_HANDLE,
	                            &vk.image_index);
	vk_assert(res);

	/* Start recording */
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

	/* Begin render pass */
	clear_values[0].color.float32[0] = 0.094f;
	clear_values[0].color.float32[1] = 0.094f;
	clear_values[0].color.float32[2] = 0.094f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[0].depthStencil.depth = 0.0f;
	clear_values[0].depthStencil.stencil = 0.0f;

	clear_values[1].color.float32[0] = 0.0f;
	clear_values[1].color.float32[1] = 0.0f;
	clear_values[1].color.float32[2] = 0.0f;
	clear_values[1].color.float32[3] = 0.0f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0.0f;

	pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pass_begin.pNext = NULL;
	pass_begin.renderPass = vk.render_pass;
	pass_begin.framebuffer = vk.frame_buffers[vk.image_index];
	pass_begin.renderArea.offset.x = 0;
	pass_begin.renderArea.offset.y = 0;
	pass_begin.renderArea.extent = vk.win_size;
	pass_begin.clearValueCount = 2;
	pass_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(vk.command_buffer, &pass_begin,
	                     VK_SUBPASS_CONTENTS_INLINE);

	/* Set the viewport and flip it for opengl compatibility */
	height = vk.win_size.height;

	viewport.x = 0;
	viewport.y = vk.win_size.height;
	viewport.width = vk.win_size.width;
	viewport.height = -height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(vk.command_buffer, 0, 1, &viewport);

	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = vk.win_size;

	vkCmdSetScissor(vk.command_buffer, 0, 1, &scissor);

	return 0;
}


extern void vk_render_set_pipeline(struct vk_pipeline pipeline)
{
	vkCmdBindPipeline(vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  pipeline.pipeline);
}


extern void vk_render_set_constant_data(struct vk_pipeline pipeline,
                                        VkDescriptorSet set)
{
	vkCmdBindDescriptorSets(vk.command_buffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        pipeline.layout, 0, 1, &set, 0, NULL);
}


extern void vk_render_set_vertex_buffer(struct vk_buffer buffer)
{
	VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(vk.command_buffer, 0, 1, &buffer.buffer,
	                       &offset);
}


extern void vk_render_set_index_buffer(struct vk_buffer buffer)
{
	vkCmdBindIndexBuffer(vk.command_buffer, buffer.buffer, 0,
	                     VK_INDEX_TYPE_UINT32);
}


extern void vk_render_draw(uint32_t index_count)
{
	vkCmdDrawIndexed(vk.command_buffer, index_count, 1, 0, 0, 1);
}


extern int vk_render_end(void)
{
	VkResult res;
	VkPipelineStageFlags wait_stage;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;

	/* End recording */
	vkCmdEndRenderPass(vk.command_buffer);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

	/* Render */
	wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &vk.image_aquired;
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vk.command_buffer;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;

	res = vkQueueSubmit(vk.queue, 1, &submit_info, vk.queue_submit);
	vk_assert(res);

	do {
		res = vkWaitForFences(vk.device, 1, &vk.queue_submit, VK_TRUE,
		                      UINT64_MAX);
	} while(res == VK_TIMEOUT);
	vk_assert(res);

	/* Display */
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = NULL;
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores = NULL;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &vk.swapchain;
	present_info.pImageIndices = &vk.image_index;
	present_info.pResults = NULL;

	res = vkQueuePresentKHR(vk.queue, &present_info);
	vk_assert(res);

	res = vkResetFences(vk.device, 1, &vk.queue_submit);
	vk_assert(res);
	return 0;
}


extern int vk_print_info(void)
{
	uint32_t i;
	VkResult res;
	uint32_t inst_ver;
	uint32_t inst_ext_count;
	VkExtensionProperties *inst_ext;
	VkPhysicalDeviceProperties2 gpu_props;
	VkPhysicalDeviceDriverProperties driver_props;
	uint32_t dev_ext_count;
	VkExtensionProperties *dev_ext;
	char *type;

	res = vkEnumerateInstanceVersion(&inst_ver);
	vk_assert(res);

	res = vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_count,
	                                             NULL);
	vk_assert(res);
	inst_ext = malloc(sizeof(VkExtensionProperties)*inst_ext_count);
	res = vkEnumerateInstanceExtensionProperties(NULL, &inst_ext_count,
	                                             inst_ext);

	gpu_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	gpu_props.pNext = &driver_props;
	driver_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
	driver_props.pNext = NULL;
	vkGetPhysicalDeviceProperties2(vk.gpu, &gpu_props);

	res = vkEnumerateDeviceExtensionProperties(vk.gpu, NULL, &dev_ext_count,
	                                           NULL);
	vk_assert(res);
	dev_ext = malloc(sizeof(VkExtensionProperties)*dev_ext_count);
	res = vkEnumerateDeviceExtensionProperties(vk.gpu, NULL, &dev_ext_count,
	                                           dev_ext);


	printf("------------------ Vulkan Info -------------------\n");

	printf("Instance Version: %d.%d.%d\n\n", VK_VERSION_MAJOR(inst_ver),
	       VK_VERSION_MINOR(inst_ver), VK_VERSION_PATCH(inst_ver));

	for(i = 0; i < inst_ext_count; i++) {
		char *surf = strrchr(inst_ext[i].extensionName, '_');
		if(strcmp(surf, "_surface") == 0) {
			printf("Instance Extension: %s ver %d\n",
			       inst_ext[i].extensionName,
			       inst_ext[i].specVersion);
		}
	}

	printf("\nGPU: %s\n", gpu_props.properties.deviceName);
	switch (gpu_props.properties.deviceType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		type = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		type = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		type = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		type = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		type = "VK_PHYSICAL_DEVICE_TYPE_CPU";
		break;
	default:
		type = "Unknown";
		break;
	}
	printf("Type: %s\n", type);
	printf("API Version: %d.%d.%d\n\n",
	       VK_VERSION_MAJOR(gpu_props.properties.apiVersion),
	       VK_VERSION_MINOR(gpu_props.properties.apiVersion),
	       VK_VERSION_PATCH(gpu_props.properties.apiVersion));

	printf("Driver: %s\n", driver_props.driverName);
	printf("Driver Info: %s\n", driver_props.driverInfo);
	printf("Conformance Version: %d.%d.%d.%d\n\n",
	       driver_props.conformanceVersion.major,
	       driver_props.conformanceVersion.minor,
	       driver_props.conformanceVersion.patch,
	       driver_props.conformanceVersion.subminor);

	for(i = 0; i < dev_ext_count; i++) {
		if(strcmp(dev_ext[i].extensionName, "VK_KHR_swapchain") == 0) {
			printf("Device Extension: %s ver %d\n",
			       dev_ext[i].extensionName,
			       dev_ext[i].specVersion);
		}
	}

	printf("--------------------------------------------------\n");

	free(dev_ext);
	free(inst_ext);
	return 0;
}
