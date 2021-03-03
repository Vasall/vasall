#include "vulkan.h"

#include "error.h"
#include "filesystem.h"
#include "window.h"

#define vk_assert(res) if(res != VK_SUCCESS) {\
		printf("[VULKAN] %s:%d : ", __FILE__, __LINE__);\
		print_error(res); return -1;}

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
 * Print the occured Vulkan Error
 * 
 * @res: the error number
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
 * Create a Vulkan Instance
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

	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = NULL;
	app_info.pApplicationName = NULL;
	app_info.applicationVersion = 0;
	app_info.pEngineName = NULL;
	app_info.engineVersion = 0;
	app_info.apiVersion = VK_API_VERSION_1_1;

	res = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	vk_assert(res);
	layers = malloc(sizeof(VkLayerProperties)*layer_count);
	res = vkEnumerateInstanceLayerProperties(&layer_count, layers);
	vk_assert(res);

	layer = NULL;

	for(i = 0; i < layer_count; i++) {
		if(strcmp(layers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0) {
			layer = "VK_LAYER_KHRONOS_validation";
			break;
		}
	}
	free(layers);

	if(SDL_Vulkan_GetInstanceExtensions(NULL, &ext_count, NULL) < 0)
		return -1;
	
	ext = malloc(sizeof(char*) * ext_count);

	if(SDL_Vulkan_GetInstanceExtensions(NULL, &ext_count, ext) < 0)
		return -1;

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
 * Create a surface to dislay the render on
 */
static int create_surface(SDL_Window *window)
{
	return SDL_Vulkan_CreateSurface(window, vk.instance, &vk.surface);
}

/*
 * Select the best physical device (gpu) to render on
 */
static int get_gpu(void)
{
	unsigned int i;
	uint32_t gpu_count;
	VkResult res;
	VkPhysicalDevice *gpus;

	res = vkEnumeratePhysicalDevices(vk.instance, &gpu_count, NULL);
	vk_assert(res);
	gpus = malloc(sizeof(VkPhysicalDevice) * gpu_count);
	res = vkEnumeratePhysicalDevices(vk.instance, &gpu_count, gpus);
	vk_assert(res);

	vk.gpu = gpus[0];
	for(i = 0; i < gpu_count; i++) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(gpus[i], &props);
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			vk.gpu = gpus[i];
	}
	free(gpus);

	return 0;
}

/*
 * Get the index of the best queue family, which renders
 */
static int get_family_index(void)
{
	unsigned int i;
	uint32_t family_count;
	VkQueueFamilyProperties* families;

	vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &family_count, NULL);
	families = malloc(sizeof(VkQueueFamilyProperties) * family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &family_count, families);

	for(i = 0; i < family_count; i++) {
		VkBool32 support;
		vkGetPhysicalDeviceSurfaceSupportKHR(vk.gpu, i, vk.surface, &support);
		if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && support) {
			vk.family = i;
			break;
		}
	}
	free(families);
	return 0;
}

/*
 * Create a logical device
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

	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = NULL;
	queue_info.flags = 0;
	queue_info.queueFamilyIndex = vk.family;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_prio;

	extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

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
 * Get the render queue
 */
static int get_queue(void)
{
	vkGetDeviceQueue(vk.device, vk.family, 0, &vk.queue);
	return 0;
}

/*
 * Select the best format
 */
static int get_format(void)
{
	uint32_t format_count;
	VkResult res;
	VkSurfaceFormatKHR* formats;

	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface,
						&format_count, NULL);
	vk_assert(res);
	formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface,
						&format_count, formats);
	
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
 * Select the best format for the depth image
 */
static int get_depth_format(void)
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
	return 0;
}

/*
 * Create a swapchain
 */
static int create_swapchain(void)
{
	int i;
	VkResult res;
	VkSurfaceCapabilitiesKHR caps;
	VkCompositeAlphaFlagBitsKHR composites[4];
	VkCompositeAlphaFlagBitsKHR composite;
	VkSwapchainCreateInfoKHR create_info;

	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.gpu, vk.surface, &caps);
	vk_assert(res);

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

	if(caps.currentExtent.width == 0xFFFFFFFF) {
		vk.win_size.width = WIN_W;
		vk.win_size.height = WIN_H;
	} else {
		vk.win_size = caps.currentExtent;
	}

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

	res = vkCreateSwapchainKHR(vk.device, &create_info, NULL, &vk.swapchain);
	vk_assert(res);
	return 0;
}

/*
 * Get the amount of swapchain images
 */
static int get_image_count(void)
{
	VkResult res;
	res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.image_count,
						NULL);
	vk_assert(res);
	return 0;
}

/*
 * Get the swapchain images
 */
static int get_swapchain_images(void)
{
	VkResult res;
	vk.images = malloc(sizeof(VkImage) * vk.image_count);
	res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.image_count,
						vk.images);
	vk_assert(res);
	return 0;
}

/*
 * Create fitting image views to the swapchain images
 */
static int create_image_view(void)
{
	unsigned int i;
	VkResult res;
	VkImageViewCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = vk.format.format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;

	vk.image_views = malloc(sizeof(VkImageView)*vk.image_count);

	for(i = 0; i < vk.image_count; i++) {
		create_info.image = vk.images[i];
		res = vkCreateImageView(vk.device, &create_info, NULL,
					&vk.image_views[i]);
		vk_assert(res);
	}
	return 0;
}

/*
 * Get the memory properties of the gpu
 */
static int get_memory_properties(void)
{
	vkGetPhysicalDeviceMemoryProperties(vk.gpu, &vk.mem_props);
	return 0;
}

/*
 * Select the best type of memory for a certain use case
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
 * Create the depth buffer
 */
static int create_depth_buffer(void)
{
	VkResult res;
	VkImageCreateInfo image_info;
	VkMemoryRequirements req;
	VkMemoryAllocateInfo alloc_info;
	VkImageViewCreateInfo view_info;

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = vk.depth_format;
	image_info.extent.width = vk.win_size.width;
	image_info.extent.height = vk.win_size.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.queueFamilyIndexCount = 1;
	image_info.pQueueFamilyIndices = &vk.family;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	res = vkCreateImage(vk.device, &image_info, NULL, &vk.depth_image);
	vk_assert(res);

	vkGetImageMemoryRequirements(vk.device, vk.depth_image, &req);

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.allocationSize = req.size;
	alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	res = vkAllocateMemory(vk.device, &alloc_info, NULL, &vk.depth_memory);
	vk_assert(res);
	res = vkBindImageMemory(vk.device, vk.depth_image, vk.depth_memory, 0);
	vk_assert(res);

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.flags = 0;
	view_info.image = vk.depth_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = vk.depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	res = vkCreateImageView(vk.device, &view_info, NULL, &vk.depth_view);
	vk_assert(res);
	return 0;
}

/*
 * Create the render pass
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

	attachments[0].flags = 0;
	attachments[0].format = vk.format.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	depth_reference.attachment = 1;
	depth_reference.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

	create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.attachmentCount = 2;
	create_info.pAttachments = attachments;
	create_info.subpassCount = 1;
	create_info.pSubpasses = &subpass;
	create_info.dependencyCount = 1;
	create_info.pDependencies = &dependency;

	res = vkCreateRenderPass(vk.device, &create_info, NULL, &vk.render_pass);
	vk_assert(res);
	return 0;
}

/*
 * Create framebuffers from the image view of the swapchain images
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
 * Create a command pool, which contains the command buffer
 */
static int create_command_pool(void)
{
	VkResult res;
	VkCommandPoolCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = vk.family;

	res = vkCreateCommandPool(vk.device, &create_info, NULL, &vk.command_pool);
	vk_assert(res);
	return 0;
}

/*
 * Allocate a command buffer, which records the render commands
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

	res = vkAllocateCommandBuffers(vk.device, &alloc_info, &vk.command_buffer);
	vk_assert(res);
	return 0;
}

/*
 * Create a descriptor pool, which contains the descriptor sets
 */
static int create_descriptor_pool(void)
{
	VkResult res;
	VkDescriptorPoolSize sizes[2];
	VkDescriptorPoolCreateInfo create_info;

	sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sizes[0].descriptorCount = 10;
	sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[1].descriptorCount = 10;

	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	create_info.maxSets = 5;
	create_info.poolSizeCount = 2;
	create_info.pPoolSizes = sizes;

	res = vkCreateDescriptorPool(vk.device, &create_info, NULL, &vk.pool);
	vk_assert(res);
	return 0;
}

/*
 * Create a semaphore
 */
static int create_semaphore(void)
{
	VkResult res;
	VkSemaphoreCreateInfo create_info;

	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;

	res = vkCreateSemaphore(vk.device, &create_info, NULL, &vk.image_aquired);
	vk_assert(res);

	return 0;
}

/*
 * Create a fence
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
 * Create a shader
 * 
 * @path: the path to the SPIR-V shader
 * @shd: a pointer to the handle of the shader module, which the function
 * 			creates
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
 * Create a descriptor set layout for the pipeline layout
 * Currently the set layout is the same for all pipelines, will be configurable
 * in the future
 * 
 * @set_layout: a pointer to the handle of the set layout, which the function
 * 				creates
 */
static int create_set_layout(VkDescriptorSetLayout *set_layout)
{
	VkResult res;
	VkDescriptorSetLayoutBinding bindings[2];
	VkDescriptorSetLayoutCreateInfo create_info;

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
 * Allocates a descriptor set, based on the descriptor set layout
 * 
 * @set_layout: a pointer to the descriptor set layout
 * @set: a pointer to the handle of the descriptor set, which the function
 * 			creates
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

/*
 * Create a pipeline layout based on the descriptor set layout
 * 
 * @set_layout: a pointer to the descriptor set layout
 * @layout: a pointer to the handle of the pipeline layput, which the function
 * 			creates
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

extern int vk_create_pipeline(char *vtx, char *frg, enum vk_in_attr attr,
							struct vk_pipeline *pipeline)
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
	VkGraphicsPipelineCreateInfo create_info;

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

	in_as.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	in_as.pNext = NULL;
	in_as.flags = 0;
	in_as.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	in_as.primitiveRestartEnable = VK_FALSE;

	tes.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tes.pNext = NULL;
	tes.flags = 0;
	tes.patchControlPoints = 0;

	view.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	view.pNext = NULL;
	view.flags = 0;
	view.viewportCount = 1;
	view.pViewports = NULL;
	view.scissorCount = 1;
	view.pScissors = NULL;

	ras.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ras.pNext = NULL;
	ras.flags = 0;
	ras.depthClampEnable = VK_FALSE;
	ras.rasterizerDiscardEnable = VK_FALSE;
	ras.polygonMode = VK_POLYGON_MODE_FILL;
	ras.cullMode = VK_CULL_MODE_BACK_BIT;
	ras.frontFace = VK_FRONT_FACE_CLOCKWISE;
	ras.depthBiasEnable = VK_FALSE;
	ras.depthBiasConstantFactor = 0.0f;
	ras.depthBiasClamp = 0.0f;
	ras.depthBiasSlopeFactor = 0.0f;
	ras.lineWidth = 1.0f;

	multi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi.pNext = NULL;
	multi.flags = 0;
	multi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multi.sampleShadingEnable = VK_FALSE;
	multi.minSampleShading = 0;
	multi.pSampleMask = NULL;
	multi.alphaToCoverageEnable = VK_FALSE;
	multi.alphaToOneEnable = VK_FALSE;

	depth.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth.pNext = NULL;
	depth.flags = 0;
	depth.depthTestEnable = VK_TRUE;
	depth.depthWriteEnable = VK_TRUE;
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

	dynamic[0] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamic[1] = VK_DYNAMIC_STATE_SCISSOR;

	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.pNext = NULL;
	dyn.flags = 0;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynamic;

	if(create_set_layout(&pipeline->set_layout) < 0) {
		res = VK_ERROR_UNKNOWN;
		goto err;
	}

	if(create_pipeline_layout(&pipeline->set_layout, &pipeline->layout) < 0) {
		res = VK_ERROR_UNKNOWN;
		goto err;
	}

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

	res = vkCreateGraphicsPipelines(vk.device, VK_NULL_HANDLE, 1, &create_info,
						NULL, &pipeline->pipeline);

err:
	vkDestroyShaderModule(vk.device, modules[0], NULL);
	vkDestroyShaderModule(vk.device, modules[1], NULL);
	vk_assert(res);
	return 0;
}

extern int vk_destroy_pipeline(struct vk_pipeline pipeline)
{
	vkDestroyPipeline(vk.device, pipeline.pipeline, NULL);
	vkDestroyPipelineLayout(vk.device, pipeline.layout, NULL);
	vkDestroyDescriptorSetLayout(vk.device, pipeline.set_layout, NULL);
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
					uint8_t staging, struct vk_buffer *buffer)
{
	VkResult res;
	VkBufferCreateInfo create_info;
	VkMemoryRequirements req;
	VkMemoryAllocateInfo alloc_info;

	buffer->size = size;

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
					&buffer->data);
		vk_assert(res);
	}

	return 0;
}

extern int vk_copy_data_to_buffer(void* data, struct vk_buffer buffer)
{
	memcpy(buffer.data, data, buffer.size);
	return 0;
}

extern int vk_destroy_buffer(struct vk_buffer buffer)
{
	vkFreeMemory(vk.device, buffer.memory, NULL);
	vkDestroyBuffer(vk.device, buffer.buffer, NULL);
	return 0;
}

extern int vk_create_texture(char *pth, struct vk_texture *texture)
{
	int w, h;
	uint8_t *buf;
	VkResult res;
	struct vk_buffer staging;
	VkImageCreateInfo image_info;
	VkMemoryRequirements req;
	VkMemoryAllocateInfo alloc_info;
	VkCommandBufferBeginInfo begin_info;
	VkImageMemoryBarrier barrier;
	VkBufferImageCopy copy;
	VkSubmitInfo submit;
	VkImageViewCreateInfo view_info;
	VkPhysicalDeviceProperties props;
	VkSamplerCreateInfo sampler_info;

	if(fs_load_png(pth, &buf, &w, &h) < 0) {
		ERR_LOG(("Failed to load texture: %s", pth));
		return -1;
	}

	if(vk_create_buffer(w*h*4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1,
				&staging) < 0) {
		ERR_LOG(("Failed to create staging buffer"));
		return -1;
	}
	memcpy(staging.data, buf, w*h*4);
	vkUnmapMemory(vk.device, staging.memory);
	free(buf);

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.flags = 0;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	image_info.extent.width = w;
	image_info.extent.height = h;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
						VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	res = vkCreateImage(vk.device, &image_info, NULL, &texture->image);
	vk_assert(res);

	vkGetImageMemoryRequirements(vk.device, texture->image, &req);

	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.allocationSize = req.size;
	alloc_info.memoryTypeIndex = get_memory_type(req.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	res = vkAllocateMemory(vk.device, &alloc_info, NULL, &texture->memory);
	vk_assert(res);

	res = vkBindImageMemory(vk.device, texture->image, texture->memory, 0);
	vk_assert(res);

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
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
			&barrier);

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

	vkCmdCopyBufferToImage(vk.command_buffer, staging.buffer, texture->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(vk.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
	submit.pSignalSemaphores = NULL;;

	res = vkQueueSubmit(vk.queue, 1, &submit, VK_NULL_HANDLE);
	vk_assert(res);

	res = vkQueueWaitIdle(vk.queue);
	vk_assert(res);

	res = vkResetCommandPool(vk.device, vk.command_pool, 0);
	vk_assert(res);

	vk_destroy_buffer(staging);

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.flags = 0;
	view_info.image = texture->image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	res = vkCreateImageView(vk.device, &view_info, NULL, &texture->image_view);
	vk_assert(res);

	vkGetPhysicalDeviceProperties(vk.gpu, &props);

	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.pNext = NULL;
	sampler_info.flags = 0;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = props.limits.maxSamplerAnisotropy;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;

	res = vkCreateSampler(vk.device, &sampler_info, NULL, &texture->sampler);
	vk_assert(res);
	return 0;
}

extern int vk_destroy_texture(struct vk_texture texture)
{
	vkDestroySampler(vk.device, texture.sampler, NULL);
	vkDestroyImageView(vk.device, texture.image_view, NULL);
	vkFreeMemory(vk.device, texture.memory, NULL);
	vkDestroyImage(vk.device, texture.image, NULL);
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

	if(get_family_index() < 0)
		goto err_surface;

	if(create_device() < 0)
		goto err_surface;

	if(get_queue() < 0)
		goto err_device;

	if(get_format() < 0)
		goto err_device;

	if(get_depth_format() < 0)
		goto err_device;

	if(create_swapchain() < 0)
		goto err_device;

	if(get_image_count() < 0)
		goto err_swapchain;

	if(get_swapchain_images() < 0)
		goto err_swapchain;

	if(create_image_view() < 0)
		goto err_swapchain_images;

	if(get_memory_properties() < 0)
		goto err_swapchain_image_views;

	if(create_depth_buffer() < 0)
		goto err_swapchain_image_views;

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
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
err_swapchain_image_views:
	for(i = 0; i < vk.image_count; i++)
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	free(vk.image_views);
err_swapchain_images:
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

extern int vk_destroy(void)
{
	uint32_t i;

	vkDestroyFence(vk.device, vk.queue_submit, NULL);
	vkDestroySemaphore(vk.device, vk.image_aquired, NULL);
	vkDestroyDescriptorPool(vk.device, vk.pool, NULL);
	vkDestroyCommandPool(vk.device, vk.command_pool, NULL);
	vkDestroyRenderPass(vk.device, vk.render_pass, NULL);
	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
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

	return 0;
}

extern int vk_resize(void)
{
	uint32_t i;

	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
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

	if(get_image_count() < 0)
		goto err_swapchain;

	if(get_swapchain_images() < 0)
		goto err_swapchain;

	if(create_image_view() < 0)
		goto err_swapchain_images;

	if(create_depth_buffer() < 0)
		goto err_swapchain_image_views;

	if(create_framebuffers() < 0)
		goto err_depth_buffer;

	return 0;

err_depth_buffer:
	vkDestroyImageView(vk.device, vk.depth_view, NULL);
	vkFreeMemory(vk.device, vk.depth_memory, NULL);
	vkDestroyImage(vk.device, vk.depth_image, NULL);
err_swapchain_image_views:
	for(i = 0; i < vk.image_count; i++)
		vkDestroyImageView(vk.device, vk.image_views[i], NULL);
	free(vk.image_views);
err_swapchain_images:
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

	res = vkAcquireNextImageKHR(vk.device, vk.swapchain, UINT64_MAX,
					vk.image_aquired, VK_NULL_HANDLE,
					&vk.image_index);
	vk_assert(res);

	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = NULL;

	res = vkBeginCommandBuffer(vk.command_buffer, &begin_info);
	vk_assert(res);

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

extern int vk_render_set_pipeline(struct vk_pipeline pipeline)
{
	vkCmdBindPipeline(vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						pipeline.pipeline);
	return 0;
}

extern int vk_render_set_constant_data(struct vk_pipeline pipeline,
							VkDescriptorSet set)
{
	vkCmdBindDescriptorSets(vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipeline.layout, 0, 1, &set, 0, NULL);
	return 0;
}

extern int vk_render_set_vertex_buffer(struct vk_buffer buffer)
{
	VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(vk.command_buffer, 0, 1, &buffer.buffer, &offset);

	return 0;
}

extern int vk_render_set_index_buffer(struct vk_buffer buffer)
{
	vkCmdBindIndexBuffer(vk.command_buffer, buffer.buffer, 0,
							VK_INDEX_TYPE_UINT32);
	return 0;
}

extern int vk_render_draw(uint32_t index_count)
{
	vkCmdDrawIndexed(vk.command_buffer, index_count, 1, 0, 0, 1);
	return 0;
}

extern int vk_render_end(void)
{
	VkResult res;
	VkPipelineStageFlags wait_stage;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;

	vkCmdEndRenderPass(vk.command_buffer);

	res = vkEndCommandBuffer(vk.command_buffer);
	vk_assert(res);

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
