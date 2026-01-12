#include "vkg.h"
#include <cassert>
#include <cstdlib>
#include <string>
#include <filesystem>
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN  // this reduces win32 headers default namespace pollution
# include <windows.h>
#else
# include <dlfcn.h>
#endif

using namespace std;
using namespace vk;


// global variables
void* vk::detail::_library = nullptr;
Instance vk::detail::_instance = nullptr;
PhysicalDevice vk::detail::_physicalDevice = nullptr;
Device vk::detail::_device = nullptr;
uint32_t vk::detail::_instanceVersion = 0;
Funcs vk::funcs;

static_assert(sizeof(vk::Instance) == sizeof(vk::Instance::HandleType), "Handle template class must not have any memory overhead and its size must be equal to encapsulated handle.");
static_assert(sizeof(vk::Device) == sizeof(vk::Device::HandleType), "Handle template class must not have any memory overhead and its size must be equal to encapsulated handle.");
static_assert(sizeof(vk::Instance) == sizeof(vk::UniqueInstance), "Handle class and its unique counterpart must be of the same size and must not have any additional memory overhead.");
static_assert(sizeof(vk::Device) == sizeof(vk::UniqueDevice), "Handle class and its unique counterpart must be of the same size and must not have any additional memory overhead.");


// init and clean up functions
// author: PCJohn (peciva at fit.vut.cz)

void vk::loadLib_throw()
{
#ifdef _WIN32
	loadLib_throw("vulkan-1.dll");
#else
	loadLib_throw("libvulkan.so.1");
#endif
}


Result vk::loadLib_noThrow() noexcept
{
#ifdef _WIN32
	return loadLib_noThrow("vulkan-1.dll");
#else
	return loadLib_noThrow("libvulkan.so.1");
#endif
}


void vk::loadLib_throw(const char* libPath)
{
	// avoid multiple initialization attempts
	if(detail::_library)
		throw VkgError("Vulkan error: Multiple initialization attempts.");

	// load library
	// and get vkGetInstanceProcAddr pointer
	filesystem::path p = filesystem::path(libPath);
#ifdef _WIN32
	detail::_library = reinterpret_cast<void*>(LoadLibraryW(p.native().c_str()));
	if(detail::_library == nullptr)
		throw VkgError((string("Vulkan error: Can not open \"") + p.string() + "\".").c_str());
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(
		GetProcAddress(reinterpret_cast<HMODULE>(detail::_library), "vkGetInstanceProcAddr"));
#else
	detail::_library = dlopen(p.native().c_str(),RTLD_NOW);
	if(detail::_library == nullptr)
		throw VkgError((string("Vulkan error: Can not open \"") + p.native() + "\".").c_str());
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(detail::_library, "vkGetInstanceProcAddr"));
#endif
	if(funcs.vkGetInstanceProcAddr == nullptr) {
		unloadLib();
		throw VkgError((string("Vulkan error: Can not retrieve vkGetInstanceProcAddr function pointer out of \"") + p.string() + ".").c_str());
	}

	// function pointers available without vk::Instance
	funcs.vkEnumerateInstanceExtensionProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
	funcs.vkEnumerateInstanceLayerProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
	funcs.vkCreateInstance = getInstanceProcAddr<PFN_vkCreateInstance>("vkCreateInstance");
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = getInstanceProcAddr<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");

	// instance version
	if(vkEnumerateInstanceVersion) {
		uint32_t v;
		Result r = vkEnumerateInstanceVersion(&v);
		if(r != Result::eSuccess) {
			unloadLib();
			throwResultException(r, "vkEnumerateInstanceVersion");
		}
		detail::_instanceVersion = v;
	}
	else
		detail::_instanceVersion = ApiVersion10;
}


Result vk::loadLib_noThrow(const char* libPath) noexcept
{
	// avoid multiple initialization attempts
	if(detail::_library)
		return Result::eErrorUnknown;

	// load library
	// and get vkGetInstanceProcAddr pointer
	filesystem::path p = filesystem::path(libPath);
#ifdef _WIN32
	detail::_library = reinterpret_cast<void*>(LoadLibraryW(p.native().c_str()));
	if(detail::_library == nullptr)
		return Result::eErrorInitializationFailed;
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(
		GetProcAddress(reinterpret_cast<HMODULE>(detail::_library), "vkGetInstanceProcAddr"));
#else
	detail::_library = dlopen(p.native().c_str(),RTLD_NOW);
	if(detail::_library == nullptr)
		return Result::eErrorInitializationFailed;
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(detail::_library, "vkGetInstanceProcAddr"));
#endif
	if(funcs.vkGetInstanceProcAddr == nullptr) {
		unloadLib();
		return Result::eErrorIncompatibleDriver;
	}

	// function pointers available without vk::Instance
	funcs.vkEnumerateInstanceExtensionProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
	funcs.vkEnumerateInstanceLayerProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
	funcs.vkCreateInstance = getInstanceProcAddr<PFN_vkCreateInstance>("vkCreateInstance");
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = getInstanceProcAddr<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");

	// instance version
	if(vkEnumerateInstanceVersion) {
		uint32_t v;
		Result r = vkEnumerateInstanceVersion(&v);
		if(r != Result::eSuccess) {
			unloadLib();
			return r;
		}
		detail::_instanceVersion = v;
	}
	else
		detail::_instanceVersion = ApiVersion10;

	return Result::eSuccess;
}


static PFN_vkGetPhysicalDeviceProperties original_vkGetPhysicalDeviceProperties;
static void VKAPI_CALL vulkan10_getPhysicalDeviceProperties(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceProperties* pProperties)
{
	original_vkGetPhysicalDeviceProperties(physicalDeviceHandle, pProperties);
	pProperties->apiVersion = vk::ApiVersion10 | vk::apiVersionPatch(pProperties->apiVersion);
}

static PFN_vkGetPhysicalDeviceProperties2 original_vkGetPhysicalDeviceProperties2;
static void VKAPI_CALL vulkan10_getPhysicalDeviceProperties2(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceProperties2* pProperties)
{
	original_vkGetPhysicalDeviceProperties2(physicalDeviceHandle, pProperties);
	pProperties->properties.apiVersion = vk::ApiVersion10 | vk::apiVersionPatch(pProperties->properties.apiVersion);
}


void vk::initInstance_throw(const vk::InstanceCreateInfo& pCreateInfo)
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance (first attempt)
	Instance::HandleType instanceHandle;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instanceHandle);

	// if creation failed, try with Vulkan 1.0 again
	bool enforceVulkan10 =
		r == Result::eErrorIncompatibleDriver &&
		pCreateInfo.pApplicationInfo &&
		pCreateInfo.pApplicationInfo->apiVersion != vk::ApiVersion10;
	if(enforceVulkan10)
	{
		// replace the requested Vulkan version by 1.0 to avoid
		// eErrorIncompatibleDriver error
		ApplicationInfo appInfo2(*pCreateInfo.pApplicationInfo);
		appInfo2.apiVersion = ApiVersion10;
		InstanceCreateInfo createInfo2(pCreateInfo);
		createInfo2.pApplicationInfo = &appInfo2;

		// create instance (second attempt)
		r = funcs.vkCreateInstance(&createInfo2, nullptr, &instanceHandle);
	}

	// test for eSuccess
	checkForSuccessValue(r, "vkCreateInstance");  // might throw

	// init instance functionality
	initInstance(instanceHandle);

	if(enforceVulkan10)
	{
		// force vkGetPhysicalDevice[Properties|Properties2] to return vk::ApiVersion10 in apiVersion
		original_vkGetPhysicalDeviceProperties = funcs.vkGetPhysicalDeviceProperties;
		original_vkGetPhysicalDeviceProperties2 = funcs.vkGetPhysicalDeviceProperties2;
		funcs.vkGetPhysicalDeviceProperties = vulkan10_getPhysicalDeviceProperties;
		funcs.vkGetPhysicalDeviceProperties2 = vulkan10_getPhysicalDeviceProperties2;
	}
}


Result vk::initInstance_noThrow(const vk::InstanceCreateInfo& pCreateInfo) noexcept
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance (first attempt)
	Instance::HandleType instanceHandle;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instanceHandle);

	// if creation failed, try with Vulkan 1.0 again
	bool enforceVulkan10 =
		r == Result::eErrorIncompatibleDriver &&
		pCreateInfo.pApplicationInfo &&
		pCreateInfo.pApplicationInfo->apiVersion != vk::ApiVersion10;
	if(enforceVulkan10)
	{
		// replace requested Vulkan version by 1.0 to avoid
		// eErrorIncompatibleDriver error
		ApplicationInfo appInfo2(*pCreateInfo.pApplicationInfo);
		appInfo2.apiVersion = ApiVersion10;
		InstanceCreateInfo createInfo2(pCreateInfo);
		createInfo2.pApplicationInfo = &appInfo2;

		// create instance (second attempt)
		r = funcs.vkCreateInstance(&createInfo2, nullptr, &instanceHandle);
	}

	// return errors
	if(r != Result::eSuccess)
		return r;

	// init instance functionality
	initInstance(instanceHandle);

	if(enforceVulkan10)
	{
		// force vkGetPhysicalDevice[Properties|Properties2] to return vk::ApiVersion10 in apiVersion
		original_vkGetPhysicalDeviceProperties = funcs.vkGetPhysicalDeviceProperties;
		original_vkGetPhysicalDeviceProperties2 = funcs.vkGetPhysicalDeviceProperties2;
		funcs.vkGetPhysicalDeviceProperties = vulkan10_getPhysicalDeviceProperties;
		funcs.vkGetPhysicalDeviceProperties2 = vulkan10_getPhysicalDeviceProperties2;
	}

	return Result::eSuccess;
}


void vk::initInstance(Instance instance) noexcept
{
	assert(detail::_library && "vk::loadLib() must be called before vk::initInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// assign new instance
	detail::_instance = instance;

	// set vkDestroyInstance
	// (this is critical otherwise we cannot destroy the instance in the case of exception or application finalization)
	funcs.vkDestroyInstance = getInstanceProcAddr<PFN_vkDestroyInstance>("vkDestroyInstance");

	// load instance function pointers
	funcs.vkEnumeratePhysicalDevices                 = getInstanceProcAddr<PFN_vkEnumeratePhysicalDevices                 >("vkEnumeratePhysicalDevices");
	funcs.vkEnumerateDeviceExtensionProperties       = getInstanceProcAddr<PFN_vkEnumerateDeviceExtensionProperties       >("vkEnumerateDeviceExtensionProperties");
	funcs.vkGetPhysicalDeviceProperties              = getInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties              >("vkGetPhysicalDeviceProperties");
	funcs.vkGetPhysicalDeviceFeatures                = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFeatures                >("vkGetPhysicalDeviceFeatures");
	funcs.vkGetPhysicalDeviceFormatProperties        = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFormatProperties        >("vkGetPhysicalDeviceFormatProperties");
	funcs.vkGetPhysicalDeviceImageFormatProperties   = getInstanceProcAddr<PFN_vkGetPhysicalDeviceImageFormatProperties   >("vkGetPhysicalDeviceImageFormatProperties");
	funcs.vkGetPhysicalDeviceMemoryProperties        = getInstanceProcAddr<PFN_vkGetPhysicalDeviceMemoryProperties        >("vkGetPhysicalDeviceMemoryProperties");
	funcs.vkGetPhysicalDeviceQueueFamilyProperties   = getInstanceProcAddr<PFN_vkGetPhysicalDeviceQueueFamilyProperties   >("vkGetPhysicalDeviceQueueFamilyProperties");
	funcs.vkGetPhysicalDeviceProperties2             = getInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2             >("vkGetPhysicalDeviceProperties2");
	funcs.vkGetPhysicalDeviceFeatures2               = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFeatures2               >("vkGetPhysicalDeviceFeatures2");
	funcs.vkGetPhysicalDeviceMemoryProperties2       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceMemoryProperties2       >("vkGetPhysicalDeviceMemoryProperties2");
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2  = getInstanceProcAddr<PFN_vkGetPhysicalDeviceQueueFamilyProperties2  >("vkGetPhysicalDeviceQueueFamilyProperties2");
	funcs.vkCreateDevice                             = getInstanceProcAddr<PFN_vkCreateDevice                             >("vkCreateDevice");
	funcs.vkDestroyDevice                            = getInstanceProcAddr<PFN_vkDestroyDevice                            >("vkDestroyDevice");
	funcs.vkGetDeviceProcAddr                        = getInstanceProcAddr<PFN_vkGetDeviceProcAddr                        >("vkGetDeviceProcAddr");
	funcs.vkGetDeviceQueue                           = getInstanceProcAddr<PFN_vkGetDeviceQueue                           >("vkGetDeviceQueue");
	funcs.vkCreateBuffer                             = getInstanceProcAddr<PFN_vkCreateBuffer                             >("vkCreateBuffer");
	funcs.vkDestroyBuffer                            = getInstanceProcAddr<PFN_vkDestroyBuffer                            >("vkDestroyBuffer");
	funcs.vkCreateBufferView                         = getInstanceProcAddr<PFN_vkCreateBufferView                         >("vkCreateBufferView");
	funcs.vkDestroyBufferView                        = getInstanceProcAddr<PFN_vkDestroyBufferView                        >("vkDestroyBufferView");
	funcs.vkEnumerateDeviceLayerProperties           = getInstanceProcAddr<PFN_vkEnumerateDeviceLayerProperties           >("vkEnumerateDeviceLayerProperties");
	funcs.vkQueueSubmit                              = getInstanceProcAddr<PFN_vkQueueSubmit                              >("vkQueueSubmit");
	funcs.vkQueueWaitIdle                            = getInstanceProcAddr<PFN_vkQueueWaitIdle                            >("vkQueueWaitIdle");
	funcs.vkDeviceWaitIdle                           = getInstanceProcAddr<PFN_vkDeviceWaitIdle                           >("vkDeviceWaitIdle");
	funcs.vkAllocateMemory                           = getInstanceProcAddr<PFN_vkAllocateMemory                           >("vkAllocateMemory");
	funcs.vkFreeMemory                               = getInstanceProcAddr<PFN_vkFreeMemory                               >("vkFreeMemory");
	funcs.vkMapMemory                                = getInstanceProcAddr<PFN_vkMapMemory                                >("vkMapMemory");
	funcs.vkUnmapMemory                              = getInstanceProcAddr<PFN_vkUnmapMemory                              >("vkUnmapMemory");
	funcs.vkFlushMappedMemoryRanges                  = getInstanceProcAddr<PFN_vkFlushMappedMemoryRanges                  >("vkFlushMappedMemoryRanges");
	funcs.vkInvalidateMappedMemoryRanges             = getInstanceProcAddr<PFN_vkInvalidateMappedMemoryRanges             >("vkInvalidateMappedMemoryRanges");
	funcs.vkGetDeviceMemoryCommitment                = getInstanceProcAddr<PFN_vkGetDeviceMemoryCommitment                >("vkGetDeviceMemoryCommitment");
	funcs.vkBindBufferMemory                         = getInstanceProcAddr<PFN_vkBindBufferMemory                         >("vkBindBufferMemory");
	funcs.vkBindImageMemory                          = getInstanceProcAddr<PFN_vkBindImageMemory                          >("vkBindImageMemory");
	funcs.vkGetBufferMemoryRequirements              = getInstanceProcAddr<PFN_vkGetBufferMemoryRequirements              >("vkGetBufferMemoryRequirements");
	funcs.vkGetImageMemoryRequirements               = getInstanceProcAddr<PFN_vkGetImageMemoryRequirements               >("vkGetImageMemoryRequirements");
	funcs.vkGetImageSparseMemoryRequirements         = getInstanceProcAddr<PFN_vkGetImageSparseMemoryRequirements         >("vkGetImageSparseMemoryRequirements");
	funcs.vkGetPhysicalDeviceSparseImageFormatProperties = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>("vkGetPhysicalDeviceSparseImageFormatProperties");
	funcs.vkQueueBindSparse                          = getInstanceProcAddr<PFN_vkQueueBindSparse                          >("vkQueueBindSparse");
	funcs.vkCreateFence                              = getInstanceProcAddr<PFN_vkCreateFence                              >("vkCreateFence");
	funcs.vkDestroyFence                             = getInstanceProcAddr<PFN_vkDestroyFence                             >("vkDestroyFence");
	funcs.vkResetFences                              = getInstanceProcAddr<PFN_vkResetFences                              >("vkResetFences");
	funcs.vkGetFenceStatus                           = getInstanceProcAddr<PFN_vkGetFenceStatus                           >("vkGetFenceStatus");
	funcs.vkWaitForFences                            = getInstanceProcAddr<PFN_vkWaitForFences                            >("vkWaitForFences");
	funcs.vkCreateSemaphore                          = getInstanceProcAddr<PFN_vkCreateSemaphore                          >("vkCreateSemaphore");
	funcs.vkDestroySemaphore                         = getInstanceProcAddr<PFN_vkDestroySemaphore                         >("vkDestroySemaphore");
	funcs.vkCreateEvent                              = getInstanceProcAddr<PFN_vkCreateEvent                              >("vkCreateEvent");
	funcs.vkDestroyEvent                             = getInstanceProcAddr<PFN_vkDestroyEvent                             >("vkDestroyEvent");
	funcs.vkGetEventStatus                           = getInstanceProcAddr<PFN_vkGetEventStatus                           >("vkGetEventStatus");
	funcs.vkSetEvent                                 = getInstanceProcAddr<PFN_vkSetEvent                                 >("vkSetEvent");
	funcs.vkResetEvent                               = getInstanceProcAddr<PFN_vkResetEvent                               >("vkResetEvent");
	funcs.vkCreateQueryPool                          = getInstanceProcAddr<PFN_vkCreateQueryPool                          >("vkCreateQueryPool");
	funcs.vkDestroyQueryPool                         = getInstanceProcAddr<PFN_vkDestroyQueryPool                         >("vkDestroyQueryPool");
	funcs.vkGetQueryPoolResults                      = getInstanceProcAddr<PFN_vkGetQueryPoolResults                      >("vkGetQueryPoolResults");
	funcs.vkCreateImage                              = getInstanceProcAddr<PFN_vkCreateImage                              >("vkCreateImage");
	funcs.vkDestroyImage                             = getInstanceProcAddr<PFN_vkDestroyImage                             >("vkDestroyImage");
	funcs.vkGetImageSubresourceLayout                = getInstanceProcAddr<PFN_vkGetImageSubresourceLayout                >("vkGetImageSubresourceLayout");
	funcs.vkCreateImageView                          = getInstanceProcAddr<PFN_vkCreateImageView                          >("vkCreateImageView");
	funcs.vkDestroyImageView                         = getInstanceProcAddr<PFN_vkDestroyImageView                         >("vkDestroyImageView");
	funcs.vkCreateShaderModule                       = getInstanceProcAddr<PFN_vkCreateShaderModule                       >("vkCreateShaderModule");
	funcs.vkDestroyShaderModule                      = getInstanceProcAddr<PFN_vkDestroyShaderModule                      >("vkDestroyShaderModule");
	funcs.vkCreatePipelineCache                      = getInstanceProcAddr<PFN_vkCreatePipelineCache                      >("vkCreatePipelineCache");
	funcs.vkDestroyPipelineCache                     = getInstanceProcAddr<PFN_vkDestroyPipelineCache                     >("vkDestroyPipelineCache");
	funcs.vkGetPipelineCacheData                     = getInstanceProcAddr<PFN_vkGetPipelineCacheData                     >("vkGetPipelineCacheData");
	funcs.vkMergePipelineCaches                      = getInstanceProcAddr<PFN_vkMergePipelineCaches                      >("vkMergePipelineCaches");
	funcs.vkCreateGraphicsPipelines                  = getInstanceProcAddr<PFN_vkCreateGraphicsPipelines                  >("vkCreateGraphicsPipelines");
	funcs.vkCreateComputePipelines                   = getInstanceProcAddr<PFN_vkCreateComputePipelines                   >("vkCreateComputePipelines");
	funcs.vkDestroyPipeline                          = getInstanceProcAddr<PFN_vkDestroyPipeline                          >("vkDestroyPipeline");
	funcs.vkCreatePipelineLayout                     = getInstanceProcAddr<PFN_vkCreatePipelineLayout                     >("vkCreatePipelineLayout");
	funcs.vkDestroyPipelineLayout                    = getInstanceProcAddr<PFN_vkDestroyPipelineLayout                    >("vkDestroyPipelineLayout");
	funcs.vkCreateSampler                            = getInstanceProcAddr<PFN_vkCreateSampler                            >("vkCreateSampler");
	funcs.vkDestroySampler                           = getInstanceProcAddr<PFN_vkDestroySampler                           >("vkDestroySampler");
	funcs.vkCreateDescriptorSetLayout                = getInstanceProcAddr<PFN_vkCreateDescriptorSetLayout                >("vkCreateDescriptorSetLayout");
	funcs.vkDestroyDescriptorSetLayout               = getInstanceProcAddr<PFN_vkDestroyDescriptorSetLayout               >("vkDestroyDescriptorSetLayout");
	funcs.vkCreateDescriptorPool                     = getInstanceProcAddr<PFN_vkCreateDescriptorPool                     >("vkCreateDescriptorPool");
	funcs.vkDestroyDescriptorPool                    = getInstanceProcAddr<PFN_vkDestroyDescriptorPool                    >("vkDestroyDescriptorPool");
	funcs.vkResetDescriptorPool                      = getInstanceProcAddr<PFN_vkResetDescriptorPool                      >("vkResetDescriptorPool");
	funcs.vkAllocateDescriptorSets                   = getInstanceProcAddr<PFN_vkAllocateDescriptorSets                   >("vkAllocateDescriptorSets");
	funcs.vkFreeDescriptorSets                       = getInstanceProcAddr<PFN_vkFreeDescriptorSets                       >("vkFreeDescriptorSets");
	funcs.vkUpdateDescriptorSets                     = getInstanceProcAddr<PFN_vkUpdateDescriptorSets                     >("vkUpdateDescriptorSets");
	funcs.vkCreateFramebuffer                        = getInstanceProcAddr<PFN_vkCreateFramebuffer                        >("vkCreateFramebuffer");
	funcs.vkDestroyFramebuffer                       = getInstanceProcAddr<PFN_vkDestroyFramebuffer                       >("vkDestroyFramebuffer");
	funcs.vkCreateRenderPass                         = getInstanceProcAddr<PFN_vkCreateRenderPass                         >("vkCreateRenderPass");
	funcs.vkDestroyRenderPass                        = getInstanceProcAddr<PFN_vkDestroyRenderPass                        >("vkDestroyRenderPass");
	funcs.vkGetRenderAreaGranularity                 = getInstanceProcAddr<PFN_vkGetRenderAreaGranularity                 >("vkGetRenderAreaGranularity");
	funcs.vkCreateCommandPool                        = getInstanceProcAddr<PFN_vkCreateCommandPool                        >("vkCreateCommandPool");
	funcs.vkDestroyCommandPool                       = getInstanceProcAddr<PFN_vkDestroyCommandPool                       >("vkDestroyCommandPool");
	funcs.vkResetCommandPool                         = getInstanceProcAddr<PFN_vkResetCommandPool                         >("vkResetCommandPool");
	funcs.vkAllocateCommandBuffers                   = getInstanceProcAddr<PFN_vkAllocateCommandBuffers                   >("vkAllocateCommandBuffers");
	funcs.vkFreeCommandBuffers                       = getInstanceProcAddr<PFN_vkFreeCommandBuffers                       >("vkFreeCommandBuffers");
	funcs.vkBeginCommandBuffer                       = getInstanceProcAddr<PFN_vkBeginCommandBuffer                       >("vkBeginCommandBuffer");
	funcs.vkEndCommandBuffer                         = getInstanceProcAddr<PFN_vkEndCommandBuffer                         >("vkEndCommandBuffer");
	funcs.vkResetCommandBuffer                       = getInstanceProcAddr<PFN_vkResetCommandBuffer                       >("vkResetCommandBuffer");
	funcs.vkCreateDescriptorUpdateTemplate           = getInstanceProcAddr<PFN_vkCreateDescriptorUpdateTemplate           >("vkCreateDescriptorUpdateTemplate");
	funcs.vkDestroyDescriptorUpdateTemplate          = getInstanceProcAddr<PFN_vkDestroyDescriptorUpdateTemplate          >("vkDestroyDescriptorUpdateTemplate");
	funcs.vkUpdateDescriptorSetWithTemplate          = getInstanceProcAddr<PFN_vkUpdateDescriptorSetWithTemplate          >("vkUpdateDescriptorSetWithTemplate");
	funcs.vkDestroySurfaceKHR                        = getInstanceProcAddr<PFN_vkDestroySurfaceKHR                        >("vkDestroySurfaceKHR");
	funcs.vkGetPhysicalDeviceSurfaceSupportKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceSupportKHR       >("vkGetPhysicalDeviceSurfaceSupportKHR");
	funcs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR  = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR  >("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	funcs.vkGetPhysicalDeviceSurfaceFormatsKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR       >("vkGetPhysicalDeviceSurfaceFormatsKHR");
	funcs.vkGetPhysicalDeviceSurfacePresentModesKHR  = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR  >("vkGetPhysicalDeviceSurfacePresentModesKHR");
	funcs.vkCreateSwapchainKHR                       = getInstanceProcAddr<PFN_vkCreateSwapchainKHR                       >("vkCreateSwapchainKHR");
	funcs.vkDestroySwapchainKHR                      = getInstanceProcAddr<PFN_vkDestroySwapchainKHR                      >("vkDestroySwapchainKHR");
	funcs.vkGetSwapchainImagesKHR                    = getInstanceProcAddr<PFN_vkGetSwapchainImagesKHR                    >("vkGetSwapchainImagesKHR");
	funcs.vkAcquireNextImageKHR                      = getInstanceProcAddr<PFN_vkAcquireNextImageKHR                      >("vkAcquireNextImageKHR");
	funcs.vkQueuePresentKHR                          = getInstanceProcAddr<PFN_vkQueuePresentKHR                          >("vkQueuePresentKHR");
	funcs.vkGetDeviceGroupPresentCapabilitiesKHR     = getInstanceProcAddr<PFN_vkGetDeviceGroupPresentCapabilitiesKHR     >("vkGetDeviceGroupPresentCapabilitiesKHR");
	funcs.vkGetDeviceGroupSurfacePresentModesKHR     = getInstanceProcAddr<PFN_vkGetDeviceGroupSurfacePresentModesKHR     >("vkGetDeviceGroupSurfacePresentModesKHR");
	funcs.vkGetPhysicalDevicePresentRectanglesKHR    = getInstanceProcAddr<PFN_vkGetPhysicalDevicePresentRectanglesKHR    >("vkGetPhysicalDevicePresentRectanglesKHR");
	funcs.vkAcquireNextImage2KHR                     = getInstanceProcAddr<PFN_vkAcquireNextImage2KHR                     >("vkAcquireNextImage2KHR");
	funcs.vkCmdNextSubpass                           = getInstanceProcAddr<PFN_vkCmdNextSubpass                           >("vkCmdNextSubpass");
	funcs.vkCmdSetViewport                           = getInstanceProcAddr<PFN_vkCmdSetViewport                           >("vkCmdSetViewport");
	funcs.vkCmdSetScissor                            = getInstanceProcAddr<PFN_vkCmdSetScissor                            >("vkCmdSetScissor");
	funcs.vkCmdSetBlendConstants                     = getInstanceProcAddr<PFN_vkCmdSetBlendConstants                     >("vkCmdSetBlendConstants");
	funcs.vkCmdSetDepthBounds                        = getInstanceProcAddr<PFN_vkCmdSetDepthBounds                        >("vkCmdSetDepthBounds");
	funcs.vkCmdSetStencilCompareMask                 = getInstanceProcAddr<PFN_vkCmdSetStencilCompareMask                 >("vkCmdSetStencilCompareMask");
	funcs.vkCmdSetStencilWriteMask                   = getInstanceProcAddr<PFN_vkCmdSetStencilWriteMask                   >("vkCmdSetStencilWriteMask");
	funcs.vkCmdSetStencilReference                   = getInstanceProcAddr<PFN_vkCmdSetStencilReference                   >("vkCmdSetStencilReference");
	funcs.vkCmdCopyImage                             = getInstanceProcAddr<PFN_vkCmdCopyImage                             >("vkCmdCopyImage");
	funcs.vkCmdBlitImage                             = getInstanceProcAddr<PFN_vkCmdBlitImage                             >("vkCmdBlitImage");
	funcs.vkCmdCopyBufferToImage                     = getInstanceProcAddr<PFN_vkCmdCopyBufferToImage                     >("vkCmdCopyBufferToImage");
	funcs.vkCmdCopyImageToBuffer                     = getInstanceProcAddr<PFN_vkCmdCopyImageToBuffer                     >("vkCmdCopyImageToBuffer");
	funcs.vkCmdUpdateBuffer                          = getInstanceProcAddr<PFN_vkCmdUpdateBuffer                          >("vkCmdUpdateBuffer");
	funcs.vkCmdFillBuffer                            = getInstanceProcAddr<PFN_vkCmdFillBuffer                            >("vkCmdFillBuffer");
	funcs.vkCmdClearColorImage                       = getInstanceProcAddr<PFN_vkCmdClearColorImage                       >("vkCmdClearColorImage");
	funcs.vkCmdClearDepthStencilImage                = getInstanceProcAddr<PFN_vkCmdClearDepthStencilImage                >("vkCmdClearDepthStencilImage");
	funcs.vkCmdClearAttachments                      = getInstanceProcAddr<PFN_vkCmdClearAttachments                      >("vkCmdClearAttachments");
	funcs.vkCmdResolveImage                          = getInstanceProcAddr<PFN_vkCmdResolveImage                          >("vkCmdResolveImage");
	funcs.vkCmdSetEvent                              = getInstanceProcAddr<PFN_vkCmdSetEvent                              >("vkCmdSetEvent");
	funcs.vkCmdResetEvent                            = getInstanceProcAddr<PFN_vkCmdResetEvent                            >("vkCmdResetEvent");
	funcs.vkCmdWaitEvents                            = getInstanceProcAddr<PFN_vkCmdWaitEvents                            >("vkCmdWaitEvents");
	funcs.vkCmdBeginQuery                            = getInstanceProcAddr<PFN_vkCmdBeginQuery                            >("vkCmdBeginQuery");
	funcs.vkCmdEndQuery                              = getInstanceProcAddr<PFN_vkCmdEndQuery                              >("vkCmdEndQuery");
	funcs.vkCmdCopyQueryPoolResults                  = getInstanceProcAddr<PFN_vkCmdCopyQueryPoolResults                  >("vkCmdCopyQueryPoolResults");
	//funcs.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = getInstanceProcAddr<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
}


void vk::initDevice_throw(PhysicalDevice pd, const struct DeviceCreateInfo& createInfo)
{
	destroyDevice();
	Device::HandleType deviceHandle;
	Result r = funcs.vkCreateDevice(pd.handle(), &createInfo, nullptr, &deviceHandle);
	checkForSuccessValue(r, "vkCreateDevice");  // might throw
	initDevice(pd, deviceHandle);
}


Result vk::initDevice_noThrow(PhysicalDevice pd, const struct DeviceCreateInfo& createInfo) noexcept
{
	destroyDevice();
	Device::HandleType deviceHandle;
	Result r = funcs.vkCreateDevice(pd.handle(), &createInfo, nullptr, &deviceHandle);
	if(r != Result::eSuccess)
		return r;
	initDevice(pd, deviceHandle);
	return Result::eSuccess;
}


void vk::initDevice(PhysicalDevice physicalDevice, Device device) noexcept
{
	assert(detail::_instance && "vk::createInstance() or vk::initInstance() must be called before vk::initDevice().");

	destroyDevice();

	detail::_physicalDevice = physicalDevice;
	detail::_device = device;

	funcs.vkGetDeviceProcAddr      = getDeviceProcAddr<PFN_vkGetDeviceProcAddr  >("vkGetDeviceProcAddr");
	funcs.vkDestroyDevice          = getDeviceProcAddr<PFN_vkDestroyDevice      >("vkDestroyDevice");
	funcs.vkGetDeviceQueue         = getDeviceProcAddr<PFN_vkGetDeviceQueue     >("vkGetDeviceQueue");
	funcs.vkCreateRenderPass       = getDeviceProcAddr<PFN_vkCreateRenderPass   >("vkCreateRenderPass");
	funcs.vkDestroyRenderPass      = getDeviceProcAddr<PFN_vkDestroyRenderPass  >("vkDestroyRenderPass");
	funcs.vkCreateBuffer           = getDeviceProcAddr<PFN_vkCreateBuffer       >("vkCreateBuffer");
	funcs.vkDestroyBuffer          = getDeviceProcAddr<PFN_vkDestroyBuffer      >("vkDestroyBuffer");
	funcs.vkGetBufferDeviceAddress = getDeviceProcAddr<PFN_vkGetBufferDeviceAddress>("vkGetBufferDeviceAddress");
	funcs.vkAllocateMemory         = getDeviceProcAddr<PFN_vkAllocateMemory     >("vkAllocateMemory");
	funcs.vkBindBufferMemory       = getDeviceProcAddr<PFN_vkBindBufferMemory   >("vkBindBufferMemory");
	funcs.vkBindImageMemory        = getDeviceProcAddr<PFN_vkBindImageMemory    >("vkBindImageMemory");
	funcs.vkFreeMemory             = getDeviceProcAddr<PFN_vkFreeMemory         >("vkFreeMemory");
	funcs.vkGetBufferMemoryRequirements = getDeviceProcAddr<PFN_vkGetBufferMemoryRequirements>("vkGetBufferMemoryRequirements");
	funcs.vkGetImageMemoryRequirements = getDeviceProcAddr<PFN_vkGetImageMemoryRequirements >("vkGetImageMemoryRequirements");
	funcs.vkMapMemory              = getDeviceProcAddr<PFN_vkMapMemory          >("vkMapMemory");
	funcs.vkUnmapMemory            = getDeviceProcAddr<PFN_vkUnmapMemory        >("vkUnmapMemory");
	funcs.vkFlushMappedMemoryRanges = getDeviceProcAddr<PFN_vkFlushMappedMemoryRanges>("vkFlushMappedMemoryRanges");
	funcs.vkCreateImage            = getDeviceProcAddr<PFN_vkCreateImage        >("vkCreateImage");
	funcs.vkDestroyImage           = getDeviceProcAddr<PFN_vkDestroyImage       >("vkDestroyImage");
	funcs.vkCreateImageView        = getDeviceProcAddr<PFN_vkCreateImageView    >("vkCreateImageView");
	funcs.vkDestroyImageView       = getDeviceProcAddr<PFN_vkDestroyImageView   >("vkDestroyImageView");
	funcs.vkCreateSampler          = getDeviceProcAddr<PFN_vkCreateSampler      >("vkCreateSampler");
	funcs.vkDestroySampler         = getDeviceProcAddr<PFN_vkDestroySampler     >("vkDestroySampler");
	funcs.vkCreateFramebuffer      = getDeviceProcAddr<PFN_vkCreateFramebuffer  >("vkCreateFramebuffer");
	funcs.vkDestroyFramebuffer     = getDeviceProcAddr<PFN_vkDestroyFramebuffer >("vkDestroyFramebuffer");
	funcs.vkCreateSwapchainKHR     = getDeviceProcAddr<PFN_vkCreateSwapchainKHR >("vkCreateSwapchainKHR");
	funcs.vkDestroySwapchainKHR    = getDeviceProcAddr<PFN_vkDestroySwapchainKHR>("vkDestroySwapchainKHR");
	funcs.vkGetSwapchainImagesKHR  = getDeviceProcAddr<PFN_vkGetSwapchainImagesKHR>("vkGetSwapchainImagesKHR");
	funcs.vkAcquireNextImageKHR    = getDeviceProcAddr<PFN_vkAcquireNextImageKHR>("vkAcquireNextImageKHR");
	funcs.vkQueuePresentKHR        = getDeviceProcAddr<PFN_vkQueuePresentKHR    >("vkQueuePresentKHR");
	funcs.vkCreateShaderModule     = getDeviceProcAddr<PFN_vkCreateShaderModule >("vkCreateShaderModule");
	funcs.vkDestroyShaderModule    = getDeviceProcAddr<PFN_vkDestroyShaderModule>("vkDestroyShaderModule");
	funcs.vkCreateDescriptorSetLayout = getDeviceProcAddr<PFN_vkCreateDescriptorSetLayout>("vkCreateDescriptorSetLayout");
	funcs.vkDestroyDescriptorSetLayout = getDeviceProcAddr<PFN_vkDestroyDescriptorSetLayout>("vkDestroyDescriptorSetLayout");
	funcs.vkCreateDescriptorPool   = getDeviceProcAddr<PFN_vkCreateDescriptorPool>("vkCreateDescriptorPool");
	funcs.vkDestroyDescriptorPool  = getDeviceProcAddr<PFN_vkDestroyDescriptorPool>("vkDestroyDescriptorPool");
	funcs.vkResetDescriptorPool    = getDeviceProcAddr<PFN_vkResetDescriptorPool>("vkResetDescriptorPool");
	funcs.vkAllocateDescriptorSets = getDeviceProcAddr<PFN_vkAllocateDescriptorSets>("vkAllocateDescriptorSets");
	funcs.vkUpdateDescriptorSets   = getDeviceProcAddr<PFN_vkUpdateDescriptorSets>("vkUpdateDescriptorSets");
	funcs.vkFreeDescriptorSets     = getDeviceProcAddr<PFN_vkFreeDescriptorSets >("vkFreeDescriptorSets");
	funcs.vkCreatePipelineCache    = getDeviceProcAddr<PFN_vkCreatePipelineCache>("vkCreatePipelineCache");
	funcs.vkDestroyPipelineCache   = getDeviceProcAddr<PFN_vkDestroyPipelineCache>("vkDestroyPipelineCache");
	funcs.vkCreatePipelineLayout   = getDeviceProcAddr<PFN_vkCreatePipelineLayout>("vkCreatePipelineLayout");
	funcs.vkDestroyPipelineLayout  = getDeviceProcAddr<PFN_vkDestroyPipelineLayout>("vkDestroyPipelineLayout");
	funcs.vkCreateGraphicsPipelines = getDeviceProcAddr<PFN_vkCreateGraphicsPipelines>("vkCreateGraphicsPipelines");
	funcs.vkCreateComputePipelines = getDeviceProcAddr<PFN_vkCreateComputePipelines>("vkCreateComputePipelines");
	funcs.vkDestroyPipeline        = getDeviceProcAddr<PFN_vkDestroyPipeline    >("vkDestroyPipeline");
	funcs.vkCreateSemaphore        = getDeviceProcAddr<PFN_vkCreateSemaphore    >("vkCreateSemaphore");
	funcs.vkDestroySemaphore       = getDeviceProcAddr<PFN_vkDestroySemaphore   >("vkDestroySemaphore");
	funcs.vkCreateCommandPool      = getDeviceProcAddr<PFN_vkCreateCommandPool  >("vkCreateCommandPool");
	funcs.vkDestroyCommandPool     = getDeviceProcAddr<PFN_vkDestroyCommandPool >("vkDestroyCommandPool");
	funcs.vkAllocateCommandBuffers = getDeviceProcAddr<PFN_vkAllocateCommandBuffers>("vkAllocateCommandBuffers");
	funcs.vkFreeCommandBuffers     = getDeviceProcAddr<PFN_vkFreeCommandBuffers >("vkFreeCommandBuffers");
	funcs.vkBeginCommandBuffer     = getDeviceProcAddr<PFN_vkBeginCommandBuffer >("vkBeginCommandBuffer");
	funcs.vkEndCommandBuffer       = getDeviceProcAddr<PFN_vkEndCommandBuffer   >("vkEndCommandBuffer");
	funcs.vkResetCommandPool       = getDeviceProcAddr<PFN_vkResetCommandPool   >("vkResetCommandPool");
	funcs.vkCmdPushConstants       = getDeviceProcAddr<PFN_vkCmdPushConstants   >("vkCmdPushConstants");
	funcs.vkCmdBeginRenderPass     = getDeviceProcAddr<PFN_vkCmdBeginRenderPass >("vkCmdBeginRenderPass");
	funcs.vkCmdEndRenderPass       = getDeviceProcAddr<PFN_vkCmdEndRenderPass   >("vkCmdEndRenderPass");
	funcs.vkCmdExecuteCommands     = getDeviceProcAddr<PFN_vkCmdExecuteCommands >("vkCmdExecuteCommands");
	funcs.vkCmdCopyBuffer          = getDeviceProcAddr<PFN_vkCmdCopyBuffer      >("vkCmdCopyBuffer");
	funcs.vkCreateFence            = getDeviceProcAddr<PFN_vkCreateFence        >("vkCreateFence");
	funcs.vkDestroyFence           = getDeviceProcAddr<PFN_vkDestroyFence       >("vkDestroyFence");
	funcs.vkCmdBindPipeline        = getDeviceProcAddr<PFN_vkCmdBindPipeline    >("vkCmdBindPipeline");
	funcs.vkCmdBindDescriptorSets  = getDeviceProcAddr<PFN_vkCmdBindDescriptorSets>("vkCmdBindDescriptorSets");
	funcs.vkCmdBindIndexBuffer     = getDeviceProcAddr<PFN_vkCmdBindIndexBuffer >("vkCmdBindIndexBuffer");
	funcs.vkCmdBindVertexBuffers   = getDeviceProcAddr<PFN_vkCmdBindVertexBuffers>("vkCmdBindVertexBuffers");
	funcs.vkCmdDrawIndexedIndirect = getDeviceProcAddr<PFN_vkCmdDrawIndexedIndirect>("vkCmdDrawIndexedIndirect");
	funcs.vkCmdDrawIndexed         = getDeviceProcAddr<PFN_vkCmdDrawIndexed     >("vkCmdDrawIndexed");
	funcs.vkCmdDraw                = getDeviceProcAddr<PFN_vkCmdDraw            >("vkCmdDraw");
	funcs.vkCmdDrawIndirect        = getDeviceProcAddr<PFN_vkCmdDrawIndirect    >("vkCmdDrawIndirect");
	funcs.vkCmdDispatch            = getDeviceProcAddr<PFN_vkCmdDispatch        >("vkCmdDispatch");
	funcs.vkCmdDispatchIndirect    = getDeviceProcAddr<PFN_vkCmdDispatchIndirect>("vkCmdDispatchIndirect");
	funcs.vkCmdDispatchBase        = getDeviceProcAddr<PFN_vkCmdDispatchBase    >("vkCmdDispatchBase");
	funcs.vkCmdPipelineBarrier     = getDeviceProcAddr<PFN_vkCmdPipelineBarrier >("vkCmdPipelineBarrier");
	funcs.vkCmdSetDepthBias        = getDeviceProcAddr<PFN_vkCmdSetDepthBias    >("vkCmdSetDepthBias");
	funcs.vkCmdSetLineWidth        = getDeviceProcAddr<PFN_vkCmdSetLineWidth    >("vkCmdSetLineWidth");
	funcs.vkCmdSetLineStippleEXT   = getDeviceProcAddr<PFN_vkCmdSetLineStippleEXT>("vkCmdSetLineStippleEXT");
	funcs.vkQueueSubmit            = getDeviceProcAddr<PFN_vkQueueSubmit        >("vkQueueSubmit");
	funcs.vkWaitForFences          = getDeviceProcAddr<PFN_vkWaitForFences      >("vkWaitForFences");
	funcs.vkResetFences            = getDeviceProcAddr<PFN_vkResetFences        >("vkResetFences");
	funcs.vkQueueWaitIdle          = getDeviceProcAddr<PFN_vkQueueWaitIdle      >("vkQueueWaitIdle");
	funcs.vkDeviceWaitIdle         = getDeviceProcAddr<PFN_vkDeviceWaitIdle     >("vkDeviceWaitIdle");
	funcs.vkCmdResetQueryPool      = getDeviceProcAddr<PFN_vkCmdResetQueryPool  >("vkCmdResetQueryPool");
	funcs.vkCmdWriteTimestamp      = getDeviceProcAddr<PFN_vkCmdWriteTimestamp  >("vkCmdWriteTimestamp");
	funcs.vkGetCalibratedTimestampsEXT = getDeviceProcAddr<PFN_vkGetCalibratedTimestampsEXT>("vkGetCalibratedTimestampsEXT");
	funcs.vkCreateQueryPool        = getDeviceProcAddr<PFN_vkCreateQueryPool    >("vkCreateQueryPool");
	funcs.vkDestroyQueryPool       = getDeviceProcAddr<PFN_vkDestroyQueryPool   >("vkDestroyQueryPool");
	funcs.vkGetQueryPoolResults    = getDeviceProcAddr<PFN_vkGetQueryPoolResults>("vkGetQueryPoolResults");
}


void vk::destroyDevice() noexcept
{
	if(detail::_device) {
		funcs.vkDestroyDevice(detail::_device.handle(), nullptr);
		detail::_physicalDevice = nullptr;
		detail::_device = nullptr;
	}
}


void vk::destroyInstance() noexcept
{
	if(detail::_instance) {
		funcs.vkDestroyInstance(detail::_instance.handle(), nullptr);
		detail::_instance = nullptr;
	}
}


void vk::unloadLib() noexcept
{
	if(detail::_library) {

		// release Vulkan library
#ifdef _WIN32
		FreeLibrary(reinterpret_cast<HMODULE>(detail::_library));
#else
		dlclose(detail::_library);
#endif
		detail::_library = nullptr;
	}
}


void vk::cleanUp() noexcept
{
	destroyDevice();
	destroyInstance();
	unloadLib();
}


// convert VkResult to string
// author: PCJohn (peciva at fit.vut.cz)
constexpr const char* resultSuccessString = "Success";
constexpr const char* resultNotReadyString = "NotReady";
constexpr const char* resultTimeoutString = "Timeout";
constexpr const char* resultEventSetString = "EventSet";
constexpr const char* resultEventResetString = "EventReset";
constexpr const char* resultIncompleteString = "Incomplete";
constexpr const char* errorOutOfHostMemoryString = "OutOfHostMemory";
constexpr const char* errorOutOfDeviceMemoryString = "OutOfDeviceMemory";
constexpr const char* errorInitializationFailedString = "InitializationFailed";
constexpr const char* errorDeviceLostString = "DeviceLost";
constexpr const char* errorMemoryMapFailedString = "MemoryMapFailed";
constexpr const char* errorLayerNotPresentString = "LayerNotPresent";
constexpr const char* errorExtensionNotPresentString = "ExtensionNotPresent";
constexpr const char* errorFeatureNotPresentString = "FeatureNotPresent";
constexpr const char* errorIncompatibleDriverString = "IncompatibleDriver";
constexpr const char* errorTooManyObjectsString = "TooManyObjects";
constexpr const char* errorFormatNotSupportedString = "FormatNotSupported";
constexpr const char* errorFragmentedPoolString = "FragmentedPool";
constexpr const char* errorUnknownString = "Unknown";
constexpr const char* errorOutOfPoolMemoryString = "OutOfPoolMemory";
constexpr const char* errorInvalidExternalHandleString = "InvalidExternalHandle";
constexpr const char* errorFragmentationString = "Fragmentation";
constexpr const char* errorInvalidOpaqueCaptureAddressString = "InvalidOpaqueCaptureAddress";
constexpr const char* errorUnknownVkResultValueString = "UnknownVkResultValue";


vk::span<const char> vk::resultToString(Result result)
{
	switch(result) {
	case Result::eSuccess: return { resultSuccessString, 7 };
	case Result::eNotReady: return { resultNotReadyString, 8 };
	case Result::eTimeout: return { resultTimeoutString, 7 };
	case Result::eEventSet: return { resultEventSetString, 8 };
	case Result::eEventReset: return { resultEventResetString, 10 };
	case Result::eIncomplete: return { resultIncompleteString, 10 };
	case Result::eErrorOutOfHostMemory: return { errorOutOfHostMemoryString, 15 };
	case Result::eErrorOutOfDeviceMemory: return { errorOutOfDeviceMemoryString, 17 };
	case Result::eErrorInitializationFailed: return { errorInitializationFailedString, 20 };
	case Result::eErrorDeviceLost: return { errorDeviceLostString, 10 };
	case Result::eErrorMemoryMapFailed: return { errorMemoryMapFailedString, 15 }; 
	case Result::eErrorLayerNotPresent: return { errorLayerNotPresentString, 15 };
	case Result::eErrorExtensionNotPresent: return { errorExtensionNotPresentString, 19 };
	case Result::eErrorFeatureNotPresent: return { errorFeatureNotPresentString, 17 };
	case Result::eErrorIncompatibleDriver: return { errorIncompatibleDriverString, 18 };
	case Result::eErrorTooManyObjects: return { errorTooManyObjectsString, 14 };
	case Result::eErrorFormatNotSupported: return { errorFormatNotSupportedString, 18 };
	case Result::eErrorFragmentedPool: return { errorFragmentedPoolString, 14 };
	case Result::eErrorUnknown: return { errorUnknownString, 7 };
	case Result::eErrorOutOfPoolMemory: return { errorOutOfPoolMemoryString, 15 };
	case Result::eErrorInvalidExternalHandle: return { errorInvalidExternalHandleString, 21 };
	case Result::eErrorFragmentation: return { errorFragmentationString, 13 };
	case Result::eErrorInvalidOpaqueCaptureAddress: return { errorInvalidOpaqueCaptureAddressString, 27 };
	default: return { errorUnknownVkResultValueString, 20 };
	}
}


size_t vk::int32ToString(int32_t value, char* buffer)
{
	// print '-' for negative numbers
	size_t len;
	if(value < 0) {
		buffer[0] = '-';
		len = 1;
		value = abs(value);
	}
	else
		len = 0;

	// print numbers
	do {
		buffer[len] = value % 10 + '0';
		value = value / 10;
		len++;
	} while(value != 0);

	// terminating zero
	buffer[len] = 0;
	return len;
}


// exceptions
// author: PCJohn (peciva at fit.vut.cz)
void vk::throwResultException(Result result, const char* funcName)
{
	switch(result) {
	case vk::Result::eSuccess: throw SuccessResult(funcName, result);
	case vk::Result::eNotReady: throw NotReadyResult(funcName, result);
	case vk::Result::eTimeout: throw TimeoutResult(funcName, result);
	case vk::Result::eEventSet: throw EventSetResult(funcName, result);
	case vk::Result::eEventReset: throw EventResetResult(funcName, result);
	case vk::Result::eIncomplete: throw IncompleteResult(funcName, result);
	case vk::Result::eErrorOutOfHostMemory: throw OutOfHostMemoryError(funcName, result);
	case vk::Result::eErrorOutOfDeviceMemory: throw OutOfDeviceMemoryError(funcName, result);
	case vk::Result::eErrorInitializationFailed: throw InitializationFailedError(funcName, result);
	case vk::Result::eErrorDeviceLost: throw DeviceLostError(funcName, result);
	case vk::Result::eErrorMemoryMapFailed: throw MemoryMapFailedError(funcName, result);
	case vk::Result::eErrorLayerNotPresent: throw LayerNotPresentError(funcName, result);
	case vk::Result::eErrorExtensionNotPresent: throw ExtensionNotPresentError(funcName, result);
	case vk::Result::eErrorFeatureNotPresent: throw FeatureNotPresentError(funcName, result);
	case vk::Result::eErrorIncompatibleDriver: throw IncompatibleDriverError(funcName, result);
	case vk::Result::eErrorTooManyObjects: throw TooManyObjectsError(funcName, result);
	case vk::Result::eErrorFormatNotSupported: throw FormatNotSupportedError(funcName, result);
	case vk::Result::eErrorFragmentedPool: throw FragmentedPoolError(funcName, result);
	case vk::Result::eErrorUnknown: throw UnknownError(funcName, result);
	case vk::Result::eErrorOutOfPoolMemory: throw OutOfPoolMemoryError(funcName, result);
	case vk::Result::eErrorInvalidExternalHandle: throw InvalidExternalHandleError(funcName, result);
	case vk::Result::eErrorFragmentation: throw FragmentationError(funcName, result);
	case vk::Result::eErrorInvalidOpaqueCaptureAddress: throw InvalidOpaqueCaptureAddressError(funcName, result);
	default: throw VkgError("vk::throwResultException", result);
	}
}

void vk::throwResultExceptionWithMessage(Result result, const char* message)
{
	switch(result) {
	case vk::Result::eSuccess: throw SuccessResult(message);
	case vk::Result::eNotReady: throw NotReadyResult(message);
	case vk::Result::eTimeout: throw TimeoutResult(message);
	case vk::Result::eEventSet: throw EventSetResult(message);
	case vk::Result::eEventReset: throw EventResetResult(message);
	case vk::Result::eIncomplete: throw IncompleteResult(message);
	case vk::Result::eErrorOutOfHostMemory: throw OutOfHostMemoryError(message);
	case vk::Result::eErrorOutOfDeviceMemory: throw OutOfDeviceMemoryError(message);
	case vk::Result::eErrorInitializationFailed: throw InitializationFailedError(message);
	case vk::Result::eErrorDeviceLost: throw DeviceLostError(message);
	case vk::Result::eErrorMemoryMapFailed: throw MemoryMapFailedError(message);
	case vk::Result::eErrorLayerNotPresent: throw LayerNotPresentError(message);
	case vk::Result::eErrorExtensionNotPresent: throw ExtensionNotPresentError(message);
	case vk::Result::eErrorFeatureNotPresent: throw FeatureNotPresentError(message);
	case vk::Result::eErrorIncompatibleDriver: throw IncompatibleDriverError(message);
	case vk::Result::eErrorTooManyObjects: throw TooManyObjectsError(message);
	case vk::Result::eErrorFormatNotSupported: throw FormatNotSupportedError(message);
	case vk::Result::eErrorFragmentedPool: throw FragmentedPoolError(message);
	case vk::Result::eErrorUnknown: throw UnknownError(message);
	case vk::Result::eErrorOutOfPoolMemory: throw OutOfPoolMemoryError(message);
	case vk::Result::eErrorInvalidExternalHandle: throw InvalidExternalHandleError(message);
	case vk::Result::eErrorFragmentation: throw FragmentationError(message);
	case vk::Result::eErrorInvalidOpaqueCaptureAddress: throw InvalidOpaqueCaptureAddressError(message);
	default: throw VkgError("vk::throwResultException", result);
	}
}

Error::Error(const char* msgHeader, const char* msgBody) noexcept
{
	size_t l1 = strlen(msgHeader);
	size_t l2 = strlen(msgBody);
	_msg = new char[l1+l2+1];
	memcpy(&_msg[0], msgHeader, l1);
	memcpy(&_msg[l1], msgBody, l2+1);
}

Error::Error(const char* funcName, Result result) noexcept
{
	// get components of the message
	// (VkResult converted to string, funcName length, VkResult converted to integer string)
	vk::span<const char> resultText = resultToString(result);
	size_t funcNameLength = funcName ? strlen(funcName) : 0;
	char codeText[12];
	size_t codeLength = int32ToString(int32_t(result), codeText);
	size_t n = 14 + funcNameLength + 21 + resultText.size() + 16 + codeLength + 2 + 1;

	// construct message
	// example string: "Vulkan error: vkCreateInstance() failed with error InitializationFailed (VkResult code -3)."
	_msg = new char[n];
	memcpy(&_msg[0], "Vulkan error: ", 14);
	memcpy(&_msg[14], funcName, funcNameLength);
	size_t i = 14 + funcNameLength;
	memcpy(&_msg[i], "() failed with error ", 21);
	i += 21;
	memcpy(&_msg[i], resultText.data(), resultText.size());
	i += resultText.size();
	memcpy(&_msg[i], " (VkResult code ", 16);
	i += 16;
	memcpy(&_msg[i], codeText, codeLength);
	i += codeLength;
	memcpy(&_msg[i], ").", 3);
}


vk::vector<ExtensionProperties> vk::enumerateInstanceExtensionProperties_throw(const char* pLayerName)
{
	vk::vector<ExtensionProperties> v;
	uint32_t n;
	Result r;
	do {
		// get num extensions
		r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, nullptr);
		checkForSuccessValue(r, "vkEnumerateInstanceExtensionProperties");

		// enumerate extensions
		v.alloc(n);
		r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, v.data());
		checkSuccess(r, "vkEnumerateInstanceExtensionProperties");

	} while(r == Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumerateInstanceExtensionProperties_noThrow(const char* pLayerName, vk::vector<ExtensionProperties>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num extensions
		r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate extensions
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, v.data());
		if(int32_t(r) < 0)
			return r;

	} while(r == Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


vk::vector<LayerProperties> vk::enumerateInstanceLayerProperties_throw()
{
	vk::vector<LayerProperties> v;
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumerateInstanceLayerProperties(&n, nullptr);
		checkForSuccessValue(r, "vkEnumerateInstanceLayerProperties");

		// enumerate layers
		v.alloc(n);
		r = funcs.vkEnumerateInstanceLayerProperties(&n, v.data());
		checkSuccess(r, "vkEnumerateInstanceLayerProperties");

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumerateInstanceLayerProperties_noThrow(vk::vector<LayerProperties>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumerateInstanceLayerProperties(&n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate layers
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumerateInstanceLayerProperties(&n, v.data());
		if(int32_t(r) < 0)
			return r;

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


vk::vector<PhysicalDevice> vk::enumeratePhysicalDevices_throw()
{
	vk::vector<PhysicalDevice> v;
	uint32_t n;
	Result r;
	do {
		// get number of physical devices
		r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, nullptr);
		checkForSuccessValue(r, "vkEnumeratePhysicalDevices");

		// enumerate physical devices
		v.alloc(n);
		r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n,
			reinterpret_cast<PhysicalDevice::HandleType*>(v.data()));
		checkSuccess(r, "vkEnumeratePhysicalDevices");

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumeratePhysicalDevices_noThrow(vk::vector<PhysicalDevice>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate layers
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n,
			reinterpret_cast<PhysicalDevice::HandleType*>(v.data()));
		if(int32_t(r) < 0)
			return r;

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


vk::vector<ExtensionProperties> vk::enumerateDeviceExtensionProperties_throw(PhysicalDevice pd, const char* pLayerName)
{
	vk::vector<ExtensionProperties> v;
	uint32_t n;
	Result r;
	do {
		// get num extensions
		r = funcs.vkEnumerateDeviceExtensionProperties(pd.handle(), pLayerName, &n, nullptr);
		checkForSuccessValue(r, "vkEnumerateInstanceExtensionProperties");

		// enumerate extensions
		v.alloc(n);
		r = funcs.vkEnumerateDeviceExtensionProperties(pd.handle(), pLayerName, &n, v.data());
		checkSuccess(r, "vkEnumerateInstanceExtensionProperties");

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumerateDeviceExtensionProperties_noThrow(PhysicalDevice pd, const char* pLayerName, vk::vector<ExtensionProperties>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumerateDeviceExtensionProperties(pd.handle(), pLayerName, &n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate layers
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumerateDeviceExtensionProperties(pd.handle(), pLayerName, &n, v.data());
		if(int32_t(r) < 0)
			return r;

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


vk::vector<QueueFamilyProperties> vk::getPhysicalDeviceQueueFamilyProperties_throw(PhysicalDevice pd)
{
	vk::vector<QueueFamilyProperties> v;

	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd.handle(), &n, nullptr);

	// enumerate physical devices
	v.alloc(n);  // this might throw
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd.handle(), &n, v.data());

	return v;
}


Result vk::getPhysicalDeviceQueueFamilyProperties_noThrow(PhysicalDevice pd, vk::vector<QueueFamilyProperties>& v) noexcept
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd.handle(), &n, nullptr);

	// enumerate physical devices
	if(!v.alloc_noThrow(n));  // this might throw
		return Result::eErrorOutOfHostMemory;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd.handle(), &n, v.data());

	return Result::eSuccess;
}


vk::vector<QueueFamilyProperties2> vk::getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice pd)
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, nullptr);
	if(n == 0)
		return {};

	// QueueFamilyProperties2 list
	vk::vector<QueueFamilyProperties2> queueFamilyProperties;
	queueFamilyProperties.alloc(n);  // this might throw

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, queueFamilyProperties.data());
	return queueFamilyProperties;
}


Result vk::getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice pd, vk::vector<QueueFamilyProperties2>& queueFamilyProperties) noexcept
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, nullptr);

	// QueueFamilyProperties2 list
	if(!queueFamilyProperties.alloc_noThrow(n))
		return Result::eErrorOutOfHostMemory;

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, queueFamilyProperties.data());
	return Result::eSuccess;
}


const char* vk::to_cstr(PhysicalDeviceType v)
{
	switch(v) {
	case PhysicalDeviceType::eOther: return "Other";
	case PhysicalDeviceType::eIntegratedGpu: return "IntegratedGpu";
	case PhysicalDeviceType::eDiscreteGpu: return "DiscreteGpu";
	case PhysicalDeviceType::eVirtualGpu: return "VirtualGpu";
	case PhysicalDeviceType::eCpu: return "Cpu";
	default: return "Unknown";
	}
}


const char* vk::to_cstr(DriverId v)
{
	switch(v) {
	case DriverId::eAmdProprietary: return "AmdProprietary";
	case DriverId::eAmdOpenSource: return "AmdOpenSource";
	case DriverId::eMesaRadv: return "MesaRadv";
	case DriverId::eNvidiaProprietary: return "NvidiaProprietary";
	case DriverId::eIntelProprietaryWindows: return "IntelProprietaryWindows";
	case DriverId::eIntelOpenSourceMESA: return "IntelOpenSourceMESA";
	case DriverId::eImaginationProprietary: return "ImaginationProprietary";
	case DriverId::eQualcommProprietary: return "QualcommProprietary";
	case DriverId::eArmProprietary: return "ArmProprietary";
	case DriverId::eGoogleSwiftshader: return "GoogleSwiftshader";
	case DriverId::eGgpProprietary: return "GgpProprietary";
	case DriverId::eBroadcomProprietary: return "BroadcomProprietary";
	case DriverId::eMesaLlvmpipe: return "MesaLlvmpipe";
	case DriverId::eMoltenvk: return "Moltenvk";
	case DriverId::eCoreaviProprietary: return "CoreaviProprietary";
	case DriverId::eJuiceProprietary: return "JuiceProprietary";
	case DriverId::eVerisiliconProprietary: return "VerisiliconProprietary";
	case DriverId::eMesaTurnip: return "MesaTurnip";
	case DriverId::eMesaV3Dv: return "MesaV3Dv";
	case DriverId::eMesaPanvk: return "MesaPanvk";
	case DriverId::eSamsungProprietary: return "SamsungProprietary";
	case DriverId::eMesaVenus: return "MesaVenus";
	case DriverId::eMesaDozen: return "MesaDozen";
	case DriverId::eMesaNvk: return "MesaNvk";
	case DriverId::eImaginationOpenSourceMESA: return "ImaginationOpenSourceMESA";
	case DriverId::eMesaHoneykrisp: return "MesaHoneykrisp";
	case DriverId::eVulkanScEmulationOnVulkan: return "VulkanScEmulationOnVulkan";
	default: return "Unknown";
	}
}


vk::string_view vk::to_string_view(vk::PhysicalDeviceType v)
{
	switch(v) {
	case PhysicalDeviceType::eOther: return "Other";
	case PhysicalDeviceType::eIntegratedGpu: return "IntegratedGpu";
	case PhysicalDeviceType::eDiscreteGpu: return "DiscreteGpu";
	case PhysicalDeviceType::eVirtualGpu: return "VirtualGpu";
	case PhysicalDeviceType::eCpu: return "Cpu";
	default: return "Unknown";
	}
}
