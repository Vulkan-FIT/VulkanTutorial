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

using namespace vk;
using namespace std;


// global variables
void* vk::detail::_library = nullptr;
Instance vk::detail::_instance = nullptr;
PhysicalDevice vk::detail::_physicalDevice = nullptr;
Device vk::detail::_device = nullptr;
uint32_t vk::detail::_instanceVersion = 0;
Funcs vk::funcs;


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
			throwResultException("vkEnumerateInstanceVersion", r);
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


void vk::initInstance_throw(const vk::InstanceCreateInfo& pCreateInfo)
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance
	Instance instance;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);
	checkForSuccessValue(r, "vkCreateInstance");  // might throw

	// init instance functionality
	initInstance(instance);
}


void vk::initInstance_throw(const vk::InstanceCreateInfo& pCreateInfo, bool& vulkan10enforced)
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance (first attempt)
	Instance instance;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);

	if(r == Result::eErrorIncompatibleDriver && pCreateInfo.pApplicationInfo &&
	   pCreateInfo.pApplicationInfo->apiVersion != vk::ApiVersion10)
	{
		// replace requested Vulkan version by 1.0 to avoid
		// eErrorIncompatibleDriver error
		ApplicationInfo appInfo2(*pCreateInfo.pApplicationInfo);
		appInfo2.apiVersion = ApiVersion10;
		InstanceCreateInfo createInfo2(pCreateInfo);
		createInfo2.pApplicationInfo = &appInfo2;

		// create instance (second attempt)
		r = funcs.vkCreateInstance(&createInfo2, nullptr, &instance);
		vulkan10enforced = true;
	}
	else
		vulkan10enforced = false;

	// test for eSuccess
	checkForSuccessValue(r, "vkCreateInstance");  // might throw

	// init instance functionality
	initInstance(instance);
}


Result vk::initInstance_noThrow(const vk::InstanceCreateInfo& pCreateInfo) noexcept
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance
	Instance instance;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);
	if(r != Result::eSuccess)
		return r;

	// init instance functionality
	initInstance(instance);
	return Result::eSuccess;
}


Result vk::initInstance_noThrow(const vk::InstanceCreateInfo& pCreateInfo, bool& vulkan10enforced) noexcept
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	// destroy previous instance if any exists
	destroyInstance();

	// create instance (first attempt)
	Instance instance;
	Result r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);

	if(r == Result::eErrorIncompatibleDriver && pCreateInfo.pApplicationInfo &&
	   pCreateInfo.pApplicationInfo->apiVersion != vk::ApiVersion10)
	{
		// replace requested Vulkan version by 1.0 to avoid
		// eErrorIncompatibleDriver error
		ApplicationInfo appInfo2(*pCreateInfo.pApplicationInfo);
		appInfo2.apiVersion = ApiVersion10;
		InstanceCreateInfo createInfo2(pCreateInfo);
		createInfo2.pApplicationInfo = &appInfo2;

		// create instance (second attempt)
		r = funcs.vkCreateInstance(&createInfo2, nullptr, &instance);
		vulkan10enforced = true;
	}
	else
		vulkan10enforced = false;

	// return errors
	if(r != Result::eSuccess)
		return r;

	// init instance functionality
	initInstance(instance);
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
	//funcs.vkGetPhysicalDeviceSurfaceFormatsKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR       >("vkGetPhysicalDeviceSurfaceFormatsKHR");
	//funcs.vkGetPhysicalDeviceSurfacePresentModesKHR  = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR  >("vkGetPhysicalDeviceSurfacePresentModesKHR");
	//funcs.vkGetPhysicalDeviceSurfaceSupportKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceSupportKHR       >("vkGetPhysicalDeviceSurfaceSupportKHR");
	//funcs.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = getInstanceProcAddr<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");*/
}


void vk::initDevice_throw(PhysicalDevice pd, const struct DeviceCreateInfo& createInfo)
{
	destroyDevice();
	Device device;
	Result r = funcs.vkCreateDevice(pd, &createInfo, nullptr, &device);
	checkForSuccessValue(r, "vkCreateDevice");  // might throw
	initDevice(pd, device);
}


Result vk::initDevice_noThrow(PhysicalDevice pd, const struct DeviceCreateInfo& createInfo) noexcept
{
	destroyDevice();
	Device device;
	Result r = funcs.vkCreateDevice(pd, &createInfo, nullptr, &device);
	if(r != Result::eSuccess)
		return r;
	initDevice(pd, device);
	return Result::eSuccess;
}


void vk::initDevice(PhysicalDevice physicalDevice, Device device) noexcept
{
	assert(detail::_instance && "vk::createInstance() or vk::initInstance() must be called before vk::initDevice().");

	destroyDevice();

	detail::_physicalDevice = physicalDevice;
	detail::_device = device;

	funcs.vkGetDeviceProcAddr  = getDeviceProcAddr<PFN_vkGetDeviceProcAddr  >("vkGetDeviceProcAddr");
	funcs.vkDestroyDevice      = getDeviceProcAddr<PFN_vkDestroyDevice      >("vkDestroyDevice");
	//funcs.vkGetDeviceQueue     = getDeviceProcAddr<PFN_vkGetDeviceQueue     >("vkGetDeviceQueue");
/*	vkCreateRenderPass   =getProcAddr<PFN_vkCreateRenderPass   >("vkCreateRenderPass");
	vkDestroyRenderPass  =getProcAddr<PFN_vkDestroyRenderPass  >("vkDestroyRenderPass");
	vkCreateBuffer       =getProcAddr<PFN_vkCreateBuffer       >("vkCreateBuffer");
	vkDestroyBuffer      =getProcAddr<PFN_vkDestroyBuffer      >("vkDestroyBuffer");
	vkGetBufferDeviceAddress=getProcAddr<PFN_vkGetBufferDeviceAddress>("vkGetBufferDeviceAddress");
	vkAllocateMemory     =getProcAddr<PFN_vkAllocateMemory     >("vkAllocateMemory");
	vkBindBufferMemory   =getProcAddr<PFN_vkBindBufferMemory   >("vkBindBufferMemory");
	vkBindImageMemory    =getProcAddr<PFN_vkBindImageMemory    >("vkBindImageMemory");
	vkFreeMemory         =getProcAddr<PFN_vkFreeMemory         >("vkFreeMemory");
	vkGetBufferMemoryRequirements =getProcAddr<PFN_vkGetBufferMemoryRequirements>("vkGetBufferMemoryRequirements");
	vkGetImageMemoryRequirements  =getProcAddr<PFN_vkGetImageMemoryRequirements >("vkGetImageMemoryRequirements");
	vkMapMemory          =getProcAddr<PFN_vkMapMemory          >("vkMapMemory");
	vkUnmapMemory        =getProcAddr<PFN_vkUnmapMemory        >("vkUnmapMemory");
	vkFlushMappedMemoryRanges=getProcAddr<PFN_vkFlushMappedMemoryRanges>("vkFlushMappedMemoryRanges");
	vkCreateImage        =getProcAddr<PFN_vkCreateImage        >("vkCreateImage");
	vkDestroyImage       =getProcAddr<PFN_vkDestroyImage       >("vkDestroyImage");
	vkCreateImageView    =getProcAddr<PFN_vkCreateImageView    >("vkCreateImageView");
	vkDestroyImageView   =getProcAddr<PFN_vkDestroyImageView   >("vkDestroyImageView");
	vkCreateSampler      =getProcAddr<PFN_vkCreateSampler      >("vkCreateSampler");
	vkDestroySampler     =getProcAddr<PFN_vkDestroySampler     >("vkDestroySampler");
	vkCreateFramebuffer  =getProcAddr<PFN_vkCreateFramebuffer  >("vkCreateFramebuffer");
	vkDestroyFramebuffer =getProcAddr<PFN_vkDestroyFramebuffer >("vkDestroyFramebuffer");
	vkCreateSwapchainKHR =getProcAddr<PFN_vkCreateSwapchainKHR >("vkCreateSwapchainKHR");
	vkDestroySwapchainKHR=getProcAddr<PFN_vkDestroySwapchainKHR>("vkDestroySwapchainKHR");
	vkGetSwapchainImagesKHR=getProcAddr<PFN_vkGetSwapchainImagesKHR>("vkGetSwapchainImagesKHR");
	vkAcquireNextImageKHR=getProcAddr<PFN_vkAcquireNextImageKHR>("vkAcquireNextImageKHR");
	vkQueuePresentKHR    =getProcAddr<PFN_vkQueuePresentKHR    >("vkQueuePresentKHR");
	vkCreateShaderModule =getProcAddr<PFN_vkCreateShaderModule >("vkCreateShaderModule");
	vkDestroyShaderModule=getProcAddr<PFN_vkDestroyShaderModule>("vkDestroyShaderModule");
	vkCreateDescriptorSetLayout=getProcAddr<PFN_vkCreateDescriptorSetLayout>("vkCreateDescriptorSetLayout");
	vkDestroyDescriptorSetLayout=getProcAddr<PFN_vkDestroyDescriptorSetLayout>("vkDestroyDescriptorSetLayout");
	vkCreateDescriptorPool=getProcAddr<PFN_vkCreateDescriptorPool>("vkCreateDescriptorPool");
	vkDestroyDescriptorPool=getProcAddr<PFN_vkDestroyDescriptorPool>("vkDestroyDescriptorPool");
	vkResetDescriptorPool=getProcAddr<PFN_vkResetDescriptorPool>("vkResetDescriptorPool");
	vkAllocateDescriptorSets=getProcAddr<PFN_vkAllocateDescriptorSets>("vkAllocateDescriptorSets");
	vkUpdateDescriptorSets=getProcAddr<PFN_vkUpdateDescriptorSets>("vkUpdateDescriptorSets");
	vkFreeDescriptorSets =getProcAddr<PFN_vkFreeDescriptorSets >("vkFreeDescriptorSets");
	vkCreatePipelineCache=getProcAddr<PFN_vkCreatePipelineCache>("vkCreatePipelineCache");
	vkDestroyPipelineCache=getProcAddr<PFN_vkDestroyPipelineCache>("vkDestroyPipelineCache");
	vkCreatePipelineLayout=getProcAddr<PFN_vkCreatePipelineLayout>("vkCreatePipelineLayout");
	vkDestroyPipelineLayout=getProcAddr<PFN_vkDestroyPipelineLayout>("vkDestroyPipelineLayout");
	vkCreateGraphicsPipelines=getProcAddr<PFN_vkCreateGraphicsPipelines>("vkCreateGraphicsPipelines");
	vkCreateComputePipelines=getProcAddr<PFN_vkCreateComputePipelines>("vkCreateComputePipelines");
	vkDestroyPipeline    =getProcAddr<PFN_vkDestroyPipeline    >("vkDestroyPipeline");
	vkCreateSemaphore    =getProcAddr<PFN_vkCreateSemaphore    >("vkCreateSemaphore");
	vkDestroySemaphore   =getProcAddr<PFN_vkDestroySemaphore   >("vkDestroySemaphore");
	vkCreateCommandPool  =getProcAddr<PFN_vkCreateCommandPool  >("vkCreateCommandPool");
	vkDestroyCommandPool =getProcAddr<PFN_vkDestroyCommandPool >("vkDestroyCommandPool");
	vkAllocateCommandBuffers=getProcAddr<PFN_vkAllocateCommandBuffers>("vkAllocateCommandBuffers");
	vkFreeCommandBuffers =getProcAddr<PFN_vkFreeCommandBuffers >("vkFreeCommandBuffers");
	vkBeginCommandBuffer =getProcAddr<PFN_vkBeginCommandBuffer >("vkBeginCommandBuffer");
	vkEndCommandBuffer   =getProcAddr<PFN_vkEndCommandBuffer   >("vkEndCommandBuffer");
	vkResetCommandPool   =getProcAddr<PFN_vkResetCommandPool   >("vkResetCommandPool");
	vkCmdPushConstants   =getProcAddr<PFN_vkCmdPushConstants   >("vkCmdPushConstants");
	vkCmdBeginRenderPass =getProcAddr<PFN_vkCmdBeginRenderPass >("vkCmdBeginRenderPass");
	vkCmdEndRenderPass   =getProcAddr<PFN_vkCmdEndRenderPass   >("vkCmdEndRenderPass");
	vkCmdExecuteCommands =getProcAddr<PFN_vkCmdExecuteCommands >("vkCmdExecuteCommands");
	vkCmdCopyBuffer      =getProcAddr<PFN_vkCmdCopyBuffer      >("vkCmdCopyBuffer");
	vkCreateFence        =getProcAddr<PFN_vkCreateFence        >("vkCreateFence");
	vkDestroyFence       =getProcAddr<PFN_vkDestroyFence       >("vkDestroyFence");
	vkCmdBindPipeline    =getProcAddr<PFN_vkCmdBindPipeline    >("vkCmdBindPipeline");
	vkCmdBindDescriptorSets=getProcAddr<PFN_vkCmdBindDescriptorSets>("vkCmdBindDescriptorSets");
	vkCmdBindIndexBuffer =getProcAddr<PFN_vkCmdBindIndexBuffer >("vkCmdBindIndexBuffer");
	vkCmdBindVertexBuffers=getProcAddr<PFN_vkCmdBindVertexBuffers>("vkCmdBindVertexBuffers");
	vkCmdDrawIndexedIndirect=getProcAddr<PFN_vkCmdDrawIndexedIndirect>("vkCmdDrawIndexedIndirect");
	vkCmdDrawIndexed     =getProcAddr<PFN_vkCmdDrawIndexed     >("vkCmdDrawIndexed");
	vkCmdDraw            =getProcAddr<PFN_vkCmdDraw            >("vkCmdDraw");
	vkCmdDrawIndirect    =getProcAddr<PFN_vkCmdDrawIndirect    >("vkCmdDrawIndirect");
	vkCmdDispatch        =getProcAddr<PFN_vkCmdDispatch        >("vkCmdDispatch");
	vkCmdDispatchIndirect=getProcAddr<PFN_vkCmdDispatchIndirect>("vkCmdDispatchIndirect");
	vkCmdDispatchBase    =getProcAddr<PFN_vkCmdDispatchBase    >("vkCmdDispatchBase");
	vkCmdPipelineBarrier =getProcAddr<PFN_vkCmdPipelineBarrier >("vkCmdPipelineBarrier");
	vkCmdSetDepthBias    =getProcAddr<PFN_vkCmdSetDepthBias    >("vkCmdSetDepthBias");
	vkCmdSetLineWidth    =getProcAddr<PFN_vkCmdSetLineWidth    >("vkCmdSetLineWidth");
	vkCmdSetLineStippleEXT=getProcAddr<PFN_vkCmdSetLineStippleEXT>("vkCmdSetLineStippleEXT");
	vkQueueSubmit        =getProcAddr<PFN_vkQueueSubmit        >("vkQueueSubmit");
	vkWaitForFences      =getProcAddr<PFN_vkWaitForFences      >("vkWaitForFences");
	vkResetFences        =getProcAddr<PFN_vkResetFences        >("vkResetFences");
	vkQueueWaitIdle      =getProcAddr<PFN_vkQueueWaitIdle      >("vkQueueWaitIdle");
	vkDeviceWaitIdle     =getProcAddr<PFN_vkDeviceWaitIdle     >("vkDeviceWaitIdle");
	vkCmdResetQueryPool  =getProcAddr<PFN_vkCmdResetQueryPool  >("vkCmdResetQueryPool");
	vkCmdWriteTimestamp  =getProcAddr<PFN_vkCmdWriteTimestamp  >("vkCmdWriteTimestamp");
	vkGetCalibratedTimestampsEXT=getProcAddr<PFN_vkGetCalibratedTimestampsEXT>("vkGetCalibratedTimestampsEXT");
	vkCreateQueryPool    =getProcAddr<PFN_vkCreateQueryPool    >("vkCreateQueryPool");
	vkDestroyQueryPool   =getProcAddr<PFN_vkDestroyQueryPool   >("vkDestroyQueryPool");
	vkGetQueryPoolResults=getProcAddr<PFN_vkGetQueryPoolResults>("vkGetQueryPoolResults");*/
}


void vk::destroyDevice() noexcept
{
	if(detail::_device) {
		funcs.vkDestroyDevice(detail::_device, nullptr);
		detail::_physicalDevice = nullptr;
		detail::_device = nullptr;
	}
}


void vk::destroyInstance() noexcept
{
	if(detail::_instance) {
		funcs.vkDestroyInstance(detail::_instance, nullptr);
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


Span<const char> vk::resultToString(Result result)
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
void vk::throwResultException(const char* funcName, Result result)
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
	Span<const char> resultText = resultToString(result);
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


Vector<ExtensionProperties> vk::enumerateInstanceExtensionProperties_throw(const char* pLayerName)
{
	Vector<ExtensionProperties> v;
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


Result vk::enumerateInstanceExtensionProperties_noThrow(const char* pLayerName, Vector<ExtensionProperties>& v) noexcept
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


Vector<LayerProperties> vk::enumerateInstanceLayerProperties_throw()
{
	Vector<LayerProperties> v;
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


Result vk::enumerateInstanceLayerProperties_noThrow(Vector<LayerProperties>& v) noexcept
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


Vector<PhysicalDevice> vk::enumeratePhysicalDevices_throw()
{
	Vector<PhysicalDevice> v;
	uint32_t n;
	Result r;
	do {
		// get number of physical devices
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, nullptr);
		checkForSuccessValue(r, "vkEnumeratePhysicalDevices");

		// enumerate physical devices
		v.alloc(n);
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, v.data());
		checkSuccess(r, "vkEnumeratePhysicalDevices");

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumeratePhysicalDevices_noThrow(Vector<PhysicalDevice>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate layers
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, v.data());
		if(int32_t(r) < 0)
			return r;

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


Vector<ExtensionProperties> vk::enumerateDeviceExtensionProperties_throw(PhysicalDevice pd, const char* pLayerName)
{
	Vector<ExtensionProperties> v;
	uint32_t n;
	Result r;
	do {
		// get num extensions
		r = funcs.vkEnumerateDeviceExtensionProperties(pd, pLayerName, &n, nullptr);
		checkForSuccessValue(r, "vkEnumerateInstanceExtensionProperties");

		// enumerate extensions
		v.alloc(n);
		r = funcs.vkEnumerateDeviceExtensionProperties(pd, pLayerName, &n, v.data());
		checkSuccess(r, "vkEnumerateInstanceExtensionProperties");

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		v.resize(n);

	return v;
}


Result vk::enumerateDeviceExtensionProperties_noThrow(PhysicalDevice pd, const char* pLayerName, Vector<ExtensionProperties>& v) noexcept
{
	uint32_t n;
	Result r;
	do {
		// get num layers
		r = funcs.vkEnumerateDeviceExtensionProperties(pd, pLayerName, &n, nullptr);
		if(r != Result::eSuccess)
			return r;

		// enumerate layers
		if(!v.alloc_noThrow(n))
			return Result::eErrorOutOfHostMemory;
		r = funcs.vkEnumerateDeviceExtensionProperties(pd, pLayerName, &n, v.data());
		if(int32_t(r) < 0)
			return r;

	} while(r == vk::Result::eIncomplete);

	// number of returned items might got lower before the last get/enumerate call
	if(n != v.size())
		if(!v.resize_noThrow(n))
			return Result::eErrorOutOfHostMemory;

	return Result::eSuccess;
}


Vector<QueueFamilyProperties> vk::getPhysicalDeviceQueueFamilyProperties_throw(PhysicalDevice pd)
{
	Vector<QueueFamilyProperties> v;

	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, nullptr);

	// enumerate physical devices
	v.alloc(n);  // this might throw
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, v.data());

	return v;
}


Result vk::getPhysicalDeviceQueueFamilyProperties_noThrow(PhysicalDevice pd, Vector<QueueFamilyProperties>& v) noexcept
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, nullptr);

	// enumerate physical devices
	if(!v.alloc_noThrow(n));  // this might throw
		return Result::eErrorOutOfHostMemory;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties(pd, &n, v.data());

	return Result::eSuccess;
}


Vector<QueueFamilyProperties2> vk::getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice pd)
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd, &n, nullptr);
	if(n == 0)
		return {};

	// QueueFamilyProperties2 list
	Vector<QueueFamilyProperties2> queueFamilyProperties;
	queueFamilyProperties.alloc(n);  // this might throw

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd, &n, queueFamilyProperties.data());
	return queueFamilyProperties;
}


Result vk::getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice pd, Vector<QueueFamilyProperties2>& queueFamilyProperties) noexcept
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd, &n, nullptr);

	// QueueFamilyProperties2 list
	if(!queueFamilyProperties.alloc_noThrow(n))
		return Result::eErrorOutOfHostMemory;

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd, &n, queueFamilyProperties.data());
	return Result::eSuccess;
}
